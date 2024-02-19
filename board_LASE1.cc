#include "board_LASE1.h"
#include "../lib/oscilloscope.h"
#include "../lib/picsimlab.h"
#include "../lib/spareparts.h"

//OUTPUTS
enum {
    O_SS1,
    O_SS2,
    O_SS3,
    O_SS4,
    //LEDs
    O_LPWR,
    O_LICSP,
    O_RD0,
    O_RD1,
    O_RD2,
    O_RD3,
    O_RD4,
    O_RD5,
    O_RD6,
    O_RD7,
    //Bicolor LEDs
    O_BC1,   //Bicolor 1
    O_BCR1,    //R
    O_BCG1,    //G
    O_BC2,   //Bicolor 2
    O_BCR2,    //R
    O_BCG2,    //G
    //Switches
    O_SRB0,
    O_SRB1,
    O_SRB2,
    O_SRB3,
    O_SRB4,
    O_SRB5,
    O_SRB6,
    O_SRB7,
    //Switches - ICSP
    O_SPGC,
    O_SPGD,
    O_S5V,
    O_SVPP,
    //Push-buttons
    O_BRB0,
    O_BRB1,
    O_BRB2,    
    O_BRC0,
    O_BRC1,
    O_RST,
    //Relays    
    O_RL1,
    O_RL2,
    O_RL3,
    O_RL4,
    //7 seg disp
    O_A1,
    O_B1,
    O_C1,
    O_D1,
    O_E1,
    O_F1,
    O_G1,
    O_P1, //DISP1
    O_A2,
    O_B2,
    O_C2,
    O_D2,
    O_E2,
    O_F2,
    O_G2,
    O_P2, //DISP2
    O_A3,
    O_B3,
    O_C3,
    O_D3,
    O_E3,
    O_F3,
    O_G3,
    O_P3, //DISP3
    O_A4,
    O_B4,
    O_C4,
    O_D4,
    O_E4,
    O_F4,
    O_G4,
    O_P4, //DISP4
    //Lamp
    O_LAMP,
    //CPU (name)
    O_CPU
};

//INPUTS
enum {
    //Power
    I_PWR,
    //ICSP
    I_ICSP,
    //Switches
    I_SRB0,   
    I_SRB1,   
    I_SRB2,   
    I_SRB3,   
    I_SRB4,   
    I_SRB5,   
    I_SRB6,   
    I_SRB7,
    //Switches - ICSP
    I_SPGC,
    I_SPGD,
    I_S5V,
    I_SVPP,    
    //Push-buttons
    I_BRB0,
    I_BRB1,
    I_BRB2,    
    I_BRC0,
    I_BRC1,
    I_RST
};

cboard_LASE1::cboard_LASE1(void) : font(10, lxFONTFAMILY_TELETYPE, lxFONTSTYLE_NORMAL, lxFONTWEIGHT_BOLD) {
    Proc = "PIC18F4550"; // define o microcontrolador
    
    sound_on = 0; // buzzer desligado
    for(int i=0; i<4; i++)
        relay[i] = 0; // relés desligados

    lxImage image(PICSimLab.GetWindow()); // guarda uma imagem temporariamente
    image.LoadFile(lxGetLocalFile(PICSimLab.GetSharePath() + lxT("boards/Common/relay0.svg"))); // carrega uma imagem
    relay_i[0] = new lxBitmap(&image, PICSimLab.GetWindow()); // guarda o bitmap obtido a partir da imagem
    image.LoadFile(lxGetLocalFile(PICSimLab.GetSharePath() + lxT("boards/Common/relay1.svg"))); // carrega uma imagem
    relay_i[1] = new lxBitmap(&image, PICSimLab.GetWindow()); // guarda o bitmap obtido a partir da imagem

    image.Destroy();

    ReadMaps(); // lê os mapas de entradas e saídas

    for (int i = 0; i < 12; i++)
        dip[i] = 0; // chaves desligadas

    buzzer.Init(); // inicializa o buzzer

    if (PICSimLab.GetWindow()) {
        // medidor da lâmpada
        gauge1 = new CGauge();
        gauge1->SetFOwner(PICSimLab.GetWindow());
        gauge1->SetName(lxT("gauge1_p1"));
        gauge1->SetX(13);
        gauge1->SetY(102);
        gauge1->SetWidth(140);
        gauge1->SetHeight(20);
        gauge1->SetEnable(1);
        gauge1->SetVisible(1);
        gauge1->SetRange(100);
        gauge1->SetValue(0);
        gauge1->SetType(4);
        PICSimLab.GetWindow()->CreateChild(gauge1);
        // rótulo da lâmpada
        label1 = new CLabel();
        label1->SetFOwner(PICSimLab.GetWindow());
        label1->SetName(lxT("label1_p1"));
        label1->SetX(12);
        label1->SetY(80);
        label1->SetWidth(60);
        label1->SetHeight(20);
        label1->SetEnable(1);
        label1->SetVisible(1);
        label1->SetText(lxT("LAMP"));
        label1->SetAlign(1);
        PICSimLab.GetWindow()->CreateChild(label1);
    }
    SWBounce_init(&bounce, 5); // inicializa o bounce para os botões (5)
}

cboard_LASE1::~cboard_LASE1(void) {
    buzzer.BeepStop();
    buzzer.End();

    delete relay_i[0];
    delete relay_i[1];
    relay_i[0] = NULL;
    relay_i[1] = NULL;
    
    if (PICSimLab.GetWindow()) {
        PICSimLab.GetWindow()->DestroyChild(gauge1);
        PICSimLab.GetWindow()->DestroyChild(label1);
    }
    SWBounce_end(&bounce);
}

void cboard_LASE1::SetScale(double scale) {
    if (Scale != scale) {
        Scale = scale;
        if (relay_i[0]) {
            delete relay_i[0];
            delete relay_i[1];          

            lxImage image(PICSimLab.GetWindow());
            image.LoadFile(lxGetLocalFile(PICSimLab.GetSharePath() + lxT("boards/Common/relay0.svg")), 0, Scale, Scale);
            relay_i[0] = new lxBitmap(&image, PICSimLab.GetWindow());
            image.LoadFile(lxGetLocalFile(PICSimLab.GetSharePath() + lxT("boards/Common/relay1.svg")), 0, Scale, Scale);
            relay_i[1] = new lxBitmap(&image, PICSimLab.GetWindow());

            image.Destroy();
        }
    }
}

void cboard_LASE1::Draw(CDraw* draw) {
    int i;
    int update = 0; // verifica se houve atualização

    for (i = 0; i < outputc; i++) { // percorre todos os outputs 
        if (output[i].update) { // checa se o output precisa de uma atualização
            output[i].update = 0; // reseta a "flag" de update do output
            if (!update) // se é a primeira atualização do loop             
                draw->Canvas.Init(Scale, Scale); // inicia o desenho
            update++; // conta atualização

            if (!output[i].r) { // verifica se o output é um retângulo (raio = 0)
                draw->Canvas.SetFgColor(0, 0, 0); // preto
                switch(output[i].id) // trata cada output separadamente
                {
                    case O_A1:
                        draw->Canvas.SetBgColor(lm1[18], 0, 0);
                        break;
                    case O_B1:
                        draw->Canvas.SetBgColor(lm1[19], 0, 0);
                        break;
                    case O_C1:
                        draw->Canvas.SetBgColor(lm1[20], 0, 0);
                        break;
                    case O_D1:
                        draw->Canvas.SetBgColor(lm1[21], 0, 0);
                        break;
                    case O_E1:
                        draw->Canvas.SetBgColor(lm1[26], 0, 0);
                        break;
                    case O_F1:
                        draw->Canvas.SetBgColor(lm1[27], 0, 0);
                        break;
                    case O_G1:
                        draw->Canvas.SetBgColor(lm1[28], 0, 0);
                        break;

                    case O_A2:
                        draw->Canvas.SetBgColor(lm2[18], 0, 0);
                        break;
                    case O_B2:
                        draw->Canvas.SetBgColor(lm2[19], 0, 0);
                        break;
                    case O_C2:
                        draw->Canvas.SetBgColor(lm2[20], 0, 0);
                        break;
                    case O_D2:
                        draw->Canvas.SetBgColor(lm2[21], 0, 0);
                        break;
                    case O_E2:
                        draw->Canvas.SetBgColor(lm2[26], 0, 0);
                        break;
                    case O_F2:
                        draw->Canvas.SetBgColor(lm2[27], 0, 0);
                        break;
                    case O_G2:
                        draw->Canvas.SetBgColor(lm2[28], 0, 0);
                        break;

                    case O_A3:
                        draw->Canvas.SetBgColor(lm3[18], 0, 0);
                        break;
                    case O_B3:
                        draw->Canvas.SetBgColor(lm3[19], 0, 0);
                        break;
                    case O_C3:
                        draw->Canvas.SetBgColor(lm3[20], 0, 0);
                        break;
                    case O_D3:
                        draw->Canvas.SetBgColor(lm3[21], 0, 0);
                        break;
                    case O_E3:
                        draw->Canvas.SetBgColor(lm3[26], 0, 0);
                        break;
                    case O_F3:
                        draw->Canvas.SetBgColor(lm3[27], 0, 0);
                        break;
                    case O_G3:
                        draw->Canvas.SetBgColor(lm3[28], 0, 0);
                        break;

                    case O_A4:
                        draw->Canvas.SetBgColor(lm4[18], 0, 0);
                        break;
                    case O_B4:
                        draw->Canvas.SetBgColor(lm4[19], 0, 0);
                        break;
                    case O_C4:
                        draw->Canvas.SetBgColor(lm4[20], 0, 0);
                        break;
                    case O_D4:
                        draw->Canvas.SetBgColor(lm4[21], 0, 0);
                        break;
                    case O_E4:
                        draw->Canvas.SetBgColor(lm4[26], 0, 0);
                        break;
                    case O_F4:
                        draw->Canvas.SetBgColor(lm4[27], 0, 0);
                        break;
                    case O_G4:
                        draw->Canvas.SetBgColor(lm4[28], 0, 0);
                        break;
                    case O_SS1:
                        output_ids[O_A1]->update = 1;
                        output_ids[O_B1]->update = 1;
                        output_ids[O_C1]->update = 1;
                        output_ids[O_D1]->update = 1;
                        output_ids[O_E1]->update = 1;
                        output_ids[O_F1]->update = 1;
                        output_ids[O_G1]->update = 1;
                        output_ids[O_P1]->update = 1;
                        draw->Canvas.SetColor(10, 10, 10);
                        break;
                    case O_SS2:
                        output_ids[O_A2]->update = 1;
                        output_ids[O_B2]->update = 1;
                        output_ids[O_C2]->update = 1;
                        output_ids[O_D2]->update = 1;
                        output_ids[O_E2]->update = 1;
                        output_ids[O_F2]->update = 1;
                        output_ids[O_G2]->update = 1;
                        output_ids[O_P2]->update = 1;
                        draw->Canvas.SetColor(10, 10, 10);
                        break;
                    case O_SS3:
                        output_ids[O_A3]->update = 1;
                        output_ids[O_B3]->update = 1;
                        output_ids[O_C3]->update = 1;
                        output_ids[O_D3]->update = 1;
                        output_ids[O_E3]->update = 1;
                        output_ids[O_F3]->update = 1;
                        output_ids[O_G3]->update = 1;
                        output_ids[O_P3]->update = 1;
                        draw->Canvas.SetColor(10, 10, 10);
                        break;
                    case O_SS4:
                        output_ids[O_A4]->update = 1;
                        output_ids[O_B4]->update = 1;
                        output_ids[O_C4]->update = 1;
                        output_ids[O_D4]->update = 1;
                        output_ids[O_E4]->update = 1;
                        output_ids[O_F4]->update = 1;
                        output_ids[O_G4]->update = 1;
                        output_ids[O_P4]->update = 1;
                        draw->Canvas.SetColor(10, 10, 10);
                        break;
                    case O_BRB0:
                    case O_BRB1:
                    case O_BRB2:
                    case O_BRC0:
                    case O_BRC1:
                        draw->Canvas.SetColor(100, 100, 100);
                        draw->Canvas.Circle(1, output[i].cx, output[i].cy, 8);
                        if (p_BT[output[i].id - O_BRB0]) {
                            draw->Canvas.SetColor(15, 15, 15);
                        } else {
                            draw->Canvas.SetColor(55, 55, 55);
                        }
                        break;
                    case O_RST:
                        draw->Canvas.SetColor(100, 100, 100);
                        draw->Canvas.Circle(1, output[i].cx, output[i].cy, 8);
                        if (p_RST) {
                            draw->Canvas.SetColor(15, 15, 15);
                        } else {
                            draw->Canvas.SetColor(55, 55, 55);
                        }
                        break;                   
                    case O_CPU:
                        draw->Canvas.SetColor(26,26,26);
                        break;                    
                    case O_RL1:
                    case O_RL2:
                    case O_RL3:                        
                    case O_RL4:
                        draw->Canvas.ChangeScale(1.0, 1.0);
                        if (relay[output[i].id - O_RL1])
                            draw->Canvas.PutBitmap(relay_i[1], (output[i].x1 * Scale), (output[i].y1 * Scale));
                        else
                            draw->Canvas.PutBitmap(relay_i[0], (output[i].x1 * Scale), (output[i].y1 * Scale));
                        draw->Canvas.ChangeScale(Scale, Scale);
                        break;
                    default:
                        if ((output[i].name[0] == 'D') && (output[i].name[1] == 'P')) {
                            draw->Canvas.SetBgColor(250, 250, 250);
                            break;
                        }
                }

                if ((output[i].id >= O_BRB0) && (output[i].id <= O_BRC1)) {
                    draw->Canvas.Circle(1, output[i].cx, output[i].cy, 7); // círculo interno
                } else if (output[i].id == O_RST) {
                    draw->Canvas.Circle(1, output[i].cx, output[i].cy, 7); // círculo interno
                } else if ((output[i].name[0] == 'D') && (output[i].name[1] == 'P')) {
                    draw->Canvas.SetBgColor(70,70,70);
                    draw->Canvas.Rectangle(1, output[i].x1, output[i].y1,output[i].x2 - output[i].x1,output[i].y2 - output[i].y1); // retângulo escuro da chave
                    // retângulo claro da chave
                    if ((output[i].id >= O_SRB0) && (output[i].id <= O_SRB7)) {                                            
                        if (dip[output[i].name[5] - 0x30]) { // chave em 1
                            draw->Canvas.SetBgColor(255, 255, 255);
                            draw->Canvas.Rectangle(1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, (int)((output[i].y2 - output[i].y1) * 0.5));
                        } else { // chave em 0
                            draw->Canvas.SetBgColor(255, 255, 255);
                            draw->Canvas.Rectangle(1, output[i].x1, output[i].y1 + ((int)((output[i].y2 - output[i].y1) * 0.5)),
                                                    output[i].x2 - output[i].x1, (int)((output[i].y2 - output[i].y1) * 0.5));                            
                        }
                    } else {
                        if (dip[8 + output[i].id - O_SPGC]) { // chave em 1
                            draw->Canvas.SetBgColor(255, 255, 255);
                            draw->Canvas.Rectangle(1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, (int)((output[i].y2 - output[i].y1) * 0.5));   
                        } else { // chave em 0
                            draw->Canvas.SetBgColor(255, 255, 255);
                            draw->Canvas.Rectangle(1, output[i].x1, output[i].y1 + ((int)((output[i].y2 - output[i].y1) * 0.5)),
                                                    output[i].x2 - output[i].x1, (int)((output[i].y2 - output[i].y1) * 0.5));
                        }
                    }                                    
                } else if (output[i].id == O_CPU) {
                    draw->Canvas.SetFont(font);
                    draw->Canvas.SetColor(26, 26, 26);
                    draw->Canvas.Rectangle(1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1,
                                           output[i].y2 - output[i].y1); // retângulo de mesma cor do chip
                    draw->Canvas.SetColor(230, 230, 230);
                    draw->Canvas.RotatedText(Proc, output[i].x1 + 5, output[i].y1, 0); // desenha nome do microcontrolador
                } else if (!((output[i].id >= O_RL1) && (output[i].id <= O_RL4)))
                    draw->Canvas.Rectangle(1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1);
                // caso seja qualquer outra área, apenas desenha um retângulo
            } else {
                // indica se será desenhado um LED ou não
                // habilitado por padrão porque a maioria das áreas circulares são LEDs
                int led = 1;

                draw->Canvas.SetFgColor(55, 0, 0);

                switch (output[i].id) 
                {
                    case O_RD0:
                        if (PICSimLab.GetMcuPwr()) { // se o microcontrolador estiver ligado, valor é dado pelo pino (55-255)
                            draw->Canvas.SetBgColor((310 - pic.pins[19-1].oavalue), 0, 0);
                            break;
                        } else if (PICSimLab.GetMcuRst()) { // se estiver em reset, LED ligado (com nível lógico baixo no pino)
                            draw->Canvas.SetBgColor(255, 0, 0);
                            break;
                        } else { // não há alimentação e não está em reset, LED desligado
                            draw->Canvas.SetBgColor(55, 0, 0);
                            break;
                        }
                    case O_RD1:
                        if(PICSimLab.GetMcuPwr()) {
                            draw->Canvas.SetBgColor((310 - pic.pins[20-1].oavalue), 0, 0);
                            break;
                        } else if (PICSimLab.GetMcuRst()) {
                            draw->Canvas.SetBgColor(255, 0, 0);
                            break;
                        } else {
                            draw->Canvas.SetBgColor(55, 0, 0);
                            break;
                        }
                    case O_RD2:
                        if (PICSimLab.GetMcuPwr()) {
                            draw->Canvas.SetBgColor((310 - pic.pins[21-1].oavalue), 0, 0);
                            break;
                        } else if (PICSimLab.GetMcuRst()) {
                            draw->Canvas.SetBgColor(255, 0, 0);
                            break;
                        } else {
                            draw->Canvas.SetBgColor(55, 0, 0);
                            break;
                        }
                    case O_RD3:
                        if (PICSimLab.GetMcuPwr()) {
                            draw->Canvas.SetBgColor((310 - pic.pins[22-1].oavalue), 0, 0);
                            break;
                        } else if (PICSimLab.GetMcuRst()) {
                            draw->Canvas.SetBgColor(255, 0, 0);
                            break;
                        } else {
                            draw->Canvas.SetBgColor(55, 0, 0);
                            break;
                        }
                    case O_RD4:
                        if (PICSimLab.GetMcuPwr()) {
                            draw->Canvas.SetBgColor((310 - pic.pins[27-1].oavalue), 0, 0);
                            break;
                        } else if (PICSimLab.GetMcuRst()) {
                            draw->Canvas.SetBgColor(255, 0, 0);
                            break;
                        } else {
                            draw->Canvas.SetBgColor(55, 0, 0);
                            break;
                        }
                    case O_RD5:
                        if (PICSimLab.GetMcuPwr()) {
                            draw->Canvas.SetBgColor((310 - pic.pins[28-1].oavalue), 0, 0);
                            break;
                        } else if (PICSimLab.GetMcuRst()) {
                            draw->Canvas.SetBgColor(255, 0, 0);
                            break;
                        } else {
                            draw->Canvas.SetBgColor(55, 0, 0);
                            break;
                        }
                    case O_RD6:
                        if (PICSimLab.GetMcuPwr()) {
                            draw->Canvas.SetBgColor((310 - pic.pins[29-1].oavalue), 0, 0);
                            break;
                        } else if (PICSimLab.GetMcuRst()) {
                            draw->Canvas.SetBgColor(255, 0, 0);
                            break;
                        } else {
                            draw->Canvas.SetBgColor(55, 0, 0);
                            break;
                        }
                    case O_RD7:
                        if (PICSimLab.GetMcuPwr()) {
                            draw->Canvas.SetBgColor((310 - pic.pins[30-1].oavalue), 0, 0);
                            break;
                        } else if (PICSimLab.GetMcuRst()) {
                            draw->Canvas.SetBgColor(255, 0, 0);
                            break;
                        } else {
                            draw->Canvas.SetBgColor(55, 0, 0);
                            break;
                        }
                    case O_LPWR:
                        draw->Canvas.SetFgColor(0, 55, 0);
                        draw->Canvas.SetBgColor(0, 200 * PICSimLab.GetMcuPwr() + 55, 0);
                        break;
                    case O_LICSP:
                        draw->Canvas.SetFgColor(0, 0, 55);
                        draw->Canvas.SetBgColor( 0, 0, dip[10] * PICSimLab.GetMcuPwr() * 200 + 55);
                        break;
                    case O_P1:
                        led = 0;
                        draw->Canvas.SetBgColor(lm1[29], 0, 0);
                        break;
                    case O_P2:
                        led = 0;
                        draw->Canvas.SetBgColor(lm2[29], 0, 0);
                        break;
                    case O_P3:
                        led = 0;
                        draw->Canvas.SetBgColor(lm3[29], 0, 0);
                        break;
                    case O_P4:
                        led = 0;
                        draw->Canvas.SetBgColor(lm4[29], 0, 0);
                        break;
                    case O_LAMP:
                        draw->Canvas.SetFgColor(55, 55, 0);
                        draw->Canvas.SetBgColor(pic.pins[3-1].oavalue, pic.pins[3-1].oavalue, 0);
                        break;
                    case O_BC1:
                        draw->Canvas.SetFgColor(55, 55, 0);
                        draw->Canvas.SetBgColor(pic.pins[17-1].oavalue, pic.pins[10-1].oavalue, 0);
                        break;
                    case O_BC2:
                        draw->Canvas.SetFgColor(55, 55, 0);
                        draw->Canvas.SetBgColor(pic.pins[9-1].oavalue, pic.pins[8-1].oavalue, 0);
                        break;
                }
                if (led) {
                    // draw a LED
                    color1 = draw->Canvas.GetBgColor();
                    int r = color1.Red() - 120;
                    int g = color1.Green() - 120;
                    int b = color1.Blue() - 120;
                    if (r < 0)
                        r = 0;
                    if (g < 0)
                        g = 0;
                    if (b < 0)
                        b = 0;
                    color2.Set(r, g, b);
                    draw->Canvas.SetBgColor(color2);
                    draw->Canvas.Circle(1, output[i].x1, output[i].y1, output[i].r + 1);
                    draw->Canvas.SetBgColor(color1);
                    draw->Canvas.Circle(1, output[i].x1, output[i].y1, output[i].r - 2);
                } else
                    draw->Canvas.Circle(1, output[i].x1, output[i].y1, output[i].r - 1);                
            }
        }
    }        
    //end draw        
            
    if (update) 
    {
        draw->Canvas.End();
        draw->Update();
    }

    if ((((pic.pins[2-1].oavalue - 55) / 2) > 40) && PICSimLab.GetMcuPwr()) {    
        if (!sound_on) 
        {
            buzzer.BeepStart();
            sound_on = 1;
        }    
    } else {
        buzzer.BeepStop();
        sound_on = 0;
    }
    
    // Lamp
    gauge1->SetValue((pic.pins[3-1].oavalue - 55) / 2);           
}

void cboard_LASE1::Run_CPU(void){
    int i;
    int j;
    
    unsigned char pi, pj;
    unsigned char pinv;
    const picpin* pins;
    int bret;

    unsigned int alm[40];   // luminosidade media
    unsigned int alm1[40];  // luminosidade media display
    unsigned int alm2[40];  // luminosidade media display
    unsigned int alm3[40];  // luminosidade media display
    unsigned int alm4[40];  // luminosidade media display

    const int JUMPSTEPS = PICSimLab.GetJUMPSTEPS();
    const long int NSTEPJ = PICSimLab.GetNSTEPJ();
    const long int NSTEP = PICSimLab.GetNSTEP();
    const float RNSTEP = 200.0*pic.PINCOUNT/NSTEP;

    memset(alm, 0, 40 * sizeof(unsigned int));
    memset(alm1, 0, 40 * sizeof(unsigned int));
    memset(alm2, 0, 40 * sizeof(unsigned int));
    memset(alm3, 0, 40 * sizeof(unsigned int));
    memset(alm4, 0, 40 * sizeof(unsigned int));

    pins=pic.pins;
    if(use_spare)
        SpareParts.PreProcess();
    unsigned char p_BT_[5];
    memcpy(p_BT_, p_BT, 5);

    SWBounce_prepare(&bounce, PICSimLab.GetBoard()->MGetInstClockFreq());
    for (int pl = 0; pl < 3; pl++)
        if ((pins[33 + pl - 1].dir == PD_IN) && (pins[33 + pl - 1].value == p_BT_[pl]))
            SWBounce_bounce(&bounce, pl);            
    if ((pins[15 - 1].dir == PD_IN) && (pins[15 - 1].value != p_BT_[3]))
        SWBounce_bounce(&bounce, 3);               
    if ((pins[16 - 1].dir == PD_IN) && (pins[16 - 1].value != p_BT_[4]))
        SWBounce_bounce(&bounce, 4);

    j = JUMPSTEPS;
    pi = 0;
    if(PICSimLab.GetMcuPwr())
        for (i = 0; i < NSTEP; i++) {
            if(j >= JUMPSTEPS){
                pic_set_pin(&pic, pic.mclr, p_RST);
                if(!bounce.do_bounce){
                    pic_set_pin(&pic, 33, ((!(p_BT[0])) || dip[0]));
                    pic_set_pin(&pic, 34, ((!(p_BT[1])) || dip[1]));
                    pic_set_pin(&pic, 35, ((!(p_BT[2])) || dip[2]));
                    pic_set_pin(&pic, 15, p_BT_[3]);
                    pic_set_pin(&pic, 16, p_BT_[4]);
                }
            }
            
            if (bounce.do_bounce) {
                bret = SWBounce_process(&bounce);
                if (bret) {
                    if (bounce.bounce[0]) {
                        if (bret == 1)
                            pic_set_pin(&pic, 33, !pins[33 - 1].value);
                        else
                            pic_set_pin(&pic, 33, ((!(p_BT[0])) || dip[0]));                        
                    }
                    if (bounce.bounce[34]) {
                        if (bret == 1)
                            pic_set_pin(&pic, 34, !pins[34 - 1].value);
                        else
                            pic_set_pin(&pic, 34, ((!(p_BT[1])) || dip[1]));                        
                    }
                    if (bounce.bounce[2]) {
                        if (bret == 1)
                            pic_set_pin(&pic, 35, !pins[35 - 1].value);
                        else
                            pic_set_pin(&pic, 35, ((!(p_BT[2])) || dip[2]));                        
                    }
                    if (bounce.bounce[3]) {
                        if (bret == 1)
                            pic_set_pin(&pic, 15, !pins[15 - 1].value);
                        else
                            pic_set_pin(&pic, 15, p_BT_[3]);                        
                    }
                    if (bounce.bounce[4]) {
                        if (bret == 1)
                            pic_set_pin(&pic, 16, !pins[16 - 1].value);
                        else
                            pic_set_pin(&pic, 16, p_BT_[4]);                        
                    }
                }
            }

            if(!mplabxd_testbp())
                pic_step(&pic);
            ioupdated=pic.ioupdated;
            InstCounterInc();

            if(ioupdated){
                if(!bounce.do_bounce){
                    pic_set_pin(&pic, 33, ((!(p_BT[0])) || dip[0]));
                    pic_set_pin(&pic, 34, ((!(p_BT[1])) || dip[1]));
                    pic_set_pin(&pic, 35, ((!(p_BT[2])) || dip[2]));
                    pic_set_pin(&pic, 15, p_BT_[3]);
                    pic_set_pin(&pic, 16, p_BT_[4]);
                }
            }
            if(use_oscope)
                Oscilloscope.SetSample();
            if(use_spare)
                SpareParts.Process();

            alm[pi] += pins[pi].value;
            pi++;
            if (pi == pic.PINCOUNT)
                pi = 0;
            
            if (j >= JUMPSTEPS) {
                for (pj = 18; pj < 30; pj++) {
                    pinv = pins[pj].value;
                    if ((pinv) && (pins[22].value))
                        alm1[pj]++;
                    if ((pinv) && (pins[23].value))
                        alm2[pj]++;
                    if ((pinv) && (pins[24].value))
                        alm3[pj]++;
                    if ((pinv) && (pins[25].value))
                        alm4[pj]++;
                }

                j = -1;
            }
            j++;
            pic.ioupdated=0;
        }
    
    for (pi = 0; pi < pic.PINCOUNT; pi++) {
        if (pic.pins[pi].port == P_VDD)
            pic.pins[pi].oavalue = 255;
        else
            pic.pins[pi].oavalue = (int)((alm[pi] * RNSTEP) + 55);

        lm1[pi] = (int)(((600.0 * alm1[pi]) / NSTEPJ) + 30);
        lm2[pi] = (int)(((600.0 * alm2[pi]) / NSTEPJ) + 30);
        lm3[pi] = (int)(((600.0 * alm3[pi]) / NSTEPJ) + 30);
        lm4[pi] = (int)(((600.0 * alm4[pi]) / NSTEPJ) + 30);
        if (lm1[pi] > 255)
            lm1[pi] = 255;
        if (lm2[pi] > 255)
            lm2[pi] = 255;
        if (lm3[pi] > 255)
            lm3[pi] = 255;
        if (lm4[pi] > 255)
            lm4[pi] = 255;
    }
    
    if(use_spare)
        SpareParts.PostProcess();
    
    //Verifica necessidade de atualização
    //PORTD LEDs
    for (i = 0; i < 4; i++)        
        if (output_ids[O_RD0 + i]->value != pic.pins[19-1 + i].oavalue) {
            output_ids[O_RD0 + i]->value = pic.pins[19-1 + i].oavalue;
            output_ids[O_RD0 + i]->update = 1;

        }    
    for (i = 0; i < 4; i++)       
        if (output_ids[O_RD4 + i]->value != pic.pins[27-1 + i].oavalue) {
            output_ids[O_RD4 + i]->value = pic.pins[27-1 + i].oavalue;
            output_ids[O_RD4 + i]->update = 1;
        }        
    //Bicolor LEDs    
    if (output_ids[O_BCR1]->value != pic.pins[17-1].oavalue) {
        output_ids[O_BCR1]->value = pic.pins[17-1].oavalue;
        output_ids[O_BC1]->update = 1;    
    }
    if (output_ids[O_BCG1]->value != pic.pins[10-1].oavalue) {
        output_ids[O_BCG1]->value = pic.pins[10-1].oavalue;
        output_ids[O_BC1]->update = 1;    
    }
    if (output_ids[O_BCR2]->value != pic.pins[9-1].oavalue) {
        output_ids[O_BCR2]->value = pic.pins[9-1].oavalue;
        output_ids[O_BC2]->update = 1;    
    }
    if (output_ids[O_BCG2]->value != pic.pins[8-1].oavalue) {
        output_ids[O_BCG2]->value = pic.pins[8-1].oavalue;
        output_ids[O_BC2]->update = 1;    
    }    
    //LAMP
    if (output_ids[O_LAMP]->value != pic.pins[3 - 1].oavalue) {
        output_ids[O_LAMP]->value = pic.pins[3 - 1].oavalue;
        output_ids[O_LAMP]->update = 1;    
    }
    //Relays    
    for (i = 0; i < 4; i++) {
        if (pic.pins[4-1+i].oavalue==255 && PICSimLab.GetMcuPwr()) {
            relay[i] = 1;
            output_ids[O_RL1 + i]->update = 1;
        } else {
            relay[i] = 0;
            output_ids[O_RL1 + i]->update = 1;
        }
    }
    for (i = 0; i < 8; i++) {
        if (i < 4)
            j = i;
        else
            j = i + 4;        
        // 7s DISP
        if (output_ids[O_A1 + i]->value != lm1[18 + j]) {
            output_ids[O_A1 + i]->value = lm1[18 + j];
            output_ids[O_SS1]->update = 1;
        }
        if (output_ids[O_A2 + i]->value != lm2[18 + j]) {
            output_ids[O_A2 + i]->value = lm2[18 + j];
            output_ids[O_SS2]->update = 1;
        }
        if (output_ids[O_A3 + i]->value != lm3[18 + j]) {
            output_ids[O_A3 + i]->value = lm3[18 + j];
            output_ids[O_SS3]->update = 1;
        }
        if (output_ids[O_A4 + i]->value != lm4[18 + j]) {
            output_ids[O_A4 + i]->value = lm4[18 + j];
            output_ids[O_SS4]->update = 1;
        }
    }    
}

void cboard_LASE1::Reset(void){
    pic_reset(&pic, 1); // power on reset
    p_BT[0] = 1;
    p_BT[1] = 1;
    p_BT[2] = 1;
    p_BT[3] = 1;
    p_BT[4] = 1;

    pic_set_pin(&pic, 33, ((!(p_BT[0])) || dip[0]));
    pic_set_pin(&pic, 34, ((!(p_BT[1])) || dip[1]));
    pic_set_pin(&pic, 35, ((!(p_BT[2])) || dip[2]));
    pic_set_pin(&pic, 36, dip[3]);
    pic_set_pin(&pic, 37, dip[4]);
    pic_set_pin(&pic, 38, dip[5]);
    pic_set_pin(&pic, 39, dip[6]);
    pic_set_pin(&pic, 40, dip[7]);
    pic_set_pin(&pic, 15, p_BT[3]);
    pic_set_pin(&pic, 16, p_BT[4]);


    if (PICSimLab.GetStatusBar()) {
        if (pic.serial[0].serialfd != INVALID_SERIAL)
            PICSimLab.GetStatusBar()->SetField(
                2, lxT("Serial: ") + lxString::FromAscii(SERIALDEVICE) + lxT(":") + itoa(pic.serial[0].serialbaud) +
                       lxT("(") +
                       lxString().Format("%4.1f",
                                         fabs((100.0 * pic.serial[0].serialexbaud - 100.0 * pic.serial[0].serialbaud) /
                                              pic.serial[0].serialexbaud)) +
                       lxT("%)"));
        else
            PICSimLab.GetStatusBar()->SetField(2,
                                               lxT("Serial: ") + lxString::FromAscii(SERIALDEVICE) + lxT(" (ERROR)"));
    }

    for (int pi = 0; pi < pic.PINCOUNT; pi++) {
        lm1[pi] = 30;
        lm2[pi] = 30;
        lm3[pi] = 30;
        lm4[pi] = 30;
    }
    if (use_spare)
        SpareParts.Reset();
    RegisterRemoteControl();
}

void cboard_LASE1::RegisterRemoteControl(void){
    //Push-buttons
    input_ids[I_BRB0]->status = &p_BT[0];
    input_ids[I_BRB0]->update = &output_ids[O_BRB0]->update;
    input_ids[I_BRB1]->status = &p_BT[1];
    input_ids[I_BRB1]->update = &output_ids[O_BRB1]->update;
    input_ids[I_BRB2]->status = &p_BT[2];
    input_ids[I_BRB2]->update = &output_ids[O_BRB2]->update;
    input_ids[I_BRC0]->status = &p_BT[3];
    input_ids[I_BRC0]->update = &output_ids[O_BRC0]->update;
    input_ids[I_BRC1]->status = &p_BT[4];
    input_ids[I_BRC1]->update = &output_ids[O_BRC1]->update;
    //LEDs
    output_ids[O_RD0]->status = &pic.pins[19-1].oavalue;
    output_ids[O_RD1]->status = &pic.pins[20-1].oavalue;
    output_ids[O_RD2]->status = &pic.pins[21-1].oavalue;
    output_ids[O_RD3]->status = &pic.pins[22-1].oavalue;
    output_ids[O_RD4]->status = &pic.pins[27-1].oavalue;
    output_ids[O_RD5]->status = &pic.pins[28-1].oavalue;
    output_ids[O_RD6]->status = &pic.pins[29-1].oavalue;
    output_ids[O_RD7]->status = &pic.pins[30-1].oavalue;
    //Bicolor LEDs
    output_ids[O_BCR1]->status = &pic.pins[17-1].oavalue;
    output_ids[O_BCG1]->status = &pic.pins[10-1].oavalue;
    output_ids[O_BCR2]->status = &pic.pins[9-1].oavalue;
    output_ids[O_BCG2]->status = &pic.pins[8-1].oavalue;
    //LAMP
    output_ids[O_LAMP]->status = &pic.pins[3-1].oavalue;
    //DISP1
    output_ids[O_A1]->status = &lm1[18];
    output_ids[O_B1]->status = &lm1[19];
    output_ids[O_C1]->status = &lm1[20];
    output_ids[O_D1]->status = &lm1[21];
    output_ids[O_E1]->status = &lm1[26];
    output_ids[O_F1]->status = &lm1[27];
    output_ids[O_G1]->status = &lm1[28];
    output_ids[O_P1]->status = &lm1[29];
    //DISP2
    output_ids[O_A2]->status = &lm2[18];
    output_ids[O_B2]->status = &lm2[19];
    output_ids[O_C2]->status = &lm2[20];
    output_ids[O_D2]->status = &lm2[21];
    output_ids[O_E2]->status = &lm2[26];
    output_ids[O_F2]->status = &lm2[27];
    output_ids[O_G2]->status = &lm2[28];
    output_ids[O_P2]->status = &lm2[29];
    //DISP3
    output_ids[O_A3]->status = &lm3[18];
    output_ids[O_B3]->status = &lm3[19];
    output_ids[O_C3]->status = &lm3[20];
    output_ids[O_D3]->status = &lm3[21];
    output_ids[O_E3]->status = &lm3[26];
    output_ids[O_F3]->status = &lm3[27];
    output_ids[O_G3]->status = &lm3[28];
    output_ids[O_P3]->status = &lm3[29];
    //DISP4
    output_ids[O_A4]->status = &lm4[18];
    output_ids[O_B4]->status = &lm4[19];
    output_ids[O_C4]->status = &lm4[20];
    output_ids[O_D4]->status = &lm4[21];
    output_ids[O_E4]->status = &lm4[26];
    output_ids[O_F4]->status = &lm4[27];
    output_ids[O_G4]->status = &lm4[28];
    output_ids[O_P4]->status = &lm4[29];
}

void cboard_LASE1::EvMouseButtonPress(uint button, uint x, uint y, uint state){
    int i;
    for (i=0; i<inputc;i++) {
        if(((input[i].x1 <= x) && (input[i].x2 >= x)) && ((input[i].y1 <= y) && (input[i].y2 >= y))){
            switch (input[i].id) {
                case I_ICSP: {
                    if (dip[8]&&dip[9]&&dip[10]&&dip[11])
                    {
                        PICSimLab.OpenLoadHexFileDialog();
                        ;
                    }
                } break;
                case I_SRB0: {
                    dip[0] ^= 0x01;
                    pic_set_pin(&pic, 33, dip[0]);
                    output_ids[O_SRB0]->update = 1;
                } break;
                case I_SRB1: {
                    dip[1] ^= 0x01;
                    pic_set_pin(&pic, 34, dip[1]);
                    output_ids[O_SRB1]->update = 1;
                } break;
                case I_SRB2: {
                    dip[2] ^= 0x01;
                    pic_set_pin(&pic, 35, dip[2]);
                    output_ids[O_SRB2]->update = 1;
                } break;
                case I_SRB3: {
                    dip[3] ^= 0x01;
                    pic_set_pin(&pic, 36, dip[3]);
                    output_ids[O_SRB3]->update = 1;
                } break;
                case I_SRB4: {
                    dip[4] ^= 0x01;
                    pic_set_pin(&pic, 37, dip[4]);
                    output_ids[O_SRB4]->update = 1;
                } break;
                case I_SRB5: {
                    dip[5] ^= 0x01;
                    pic_set_pin(&pic, 38, dip[5]);
                    output_ids[O_SRB5]->update = 1;
                } break;
                case I_SRB6: {
                    dip[6] ^= 0x01;
                    pic_set_pin(&pic, 39, dip[6]);
                    output_ids[O_SRB6]->update = 1;
                } break;
                case I_SRB7: {
                    dip[7] ^= 0x01;
                    pic_set_pin(&pic, 40, dip[7]);
                    output_ids[O_SRB7]->update = 1;
                } break;
                case I_SPGC: {
                    dip[8] ^= 0x01;
                    output_ids[O_SPGC]->update = 1;
                } break;
                case I_SPGD: {
                    dip[9] ^= 0x01;
                    output_ids[O_SPGD]->update = 1;
                } break;
                case I_S5V: {
                    dip[10] ^= 0x01;
                    output_ids[O_S5V]->update = 1;
                    output_ids[O_LICSP]->update = 1;
                } break;
                case I_SVPP: {
                    dip[11] ^= 0x01;
                    output_ids[O_SVPP]->update = 1;
                } break;
                case I_PWR:{
                    if (PICSimLab.GetMcuPwr()) {
                        PICSimLab.SetMcuPwr(0);
                        Reset();
                        p_BT[0] = 1;
                        p_BT[1] = 1;
                        p_BT[2] = 1;
                        p_BT[3] = 1;
                        p_BT[4] = 1;                        
                    } else {
                        PICSimLab.SetMcuPwr(1);
                        Reset();
                    }
                    for (i = 0; i < 4; i++){
                        output_ids[O_RD0 + i]->value = pic.pins[19-1 + i].oavalue;
                        output_ids[O_RD0 + i]->update = 1;
                    }
                    for (i = 0; i < 4; i++){
                        output_ids[O_RD4 + i]->value = pic.pins[27-1 + i].oavalue;
                        output_ids[O_RD4 + i]->update = 1;
                    }
                    output_ids[O_LPWR]->update = 1;
                    output_ids[O_LICSP]->update = 1;
                } break;
                case I_RST: {
                    if (PICSimLab.GetMcuPwr() && pic_reset(&pic, -1)) {
                        PICSimLab.SetMcuPwr(0);
                        PICSimLab.SetMcuRst(1);
                    }
                    p_RST = 0;
                    for (int i = 0; i < 4; i++){
                        output_ids[O_RD0 + i]->value = pic.pins[19-1 + i].oavalue;
                        output_ids[O_RD0 + i]->update = 1;
                    }
                    for (int i = 0; i < 4; i++){
                        output_ids[O_RD4 + i]->value = pic.pins[27-1 + i].oavalue;
                        output_ids[O_RD4 + i]->update = 1;
                    }
                    output_ids[O_RST]->update = 1;
                } break;
                case I_BRB0: {
                    p_BT[0] = 0;
                    output_ids[O_BRB0]->update=1;
                } break;                
                case I_BRB1: {
                    p_BT[1] = 0;
                    output_ids[O_BRB1]->update=1;
                } break;                
                case I_BRB2: {
                    p_BT[2] = 0;
                    output_ids[O_BRB2]->update=1;
                } break;                
                case I_BRC0: {
                    p_BT[3] = 0;
                    output_ids[O_BRC0]->update=1;
                } break;                
                case I_BRC1: {
                    p_BT[4] = 0;
                    output_ids[O_BRC1]->update=1;
                } break;                
            }
        }
    }
}

void cboard_LASE1::EvMouseButtonRelease(uint button, uint x, uint y, uint state) {
    int i;

    for (i = 0; i < inputc; i++) {
        if (((input[i].x1 <= x) && (input[i].x2 >= x)) && ((input[i].y1 <= y) && (input[i].y2 >= y))) {
            switch (input[i].id){
                case I_RST: {
                    if (PICSimLab.GetMcuRst()) {
                        PICSimLab.SetMcuPwr(1);
                        PICSimLab.SetMcuRst(0);
                        if (pic_reset(&pic, -1))
                            Reset();                        
                    }
                    p_RST = 1;
                    for (int i = 0; i < 4; i++){
                        output_ids[O_RD0 + i]->value = pic.pins[19-1 + i].oavalue;
                        output_ids[O_RD0 + i]->update = 1;
                    }
                    for (int i = 0; i < 4; i++){
                        output_ids[O_RD4 + i]->value = pic.pins[27-1 + i].oavalue;
                        output_ids[O_RD4 + i]->update = 1;
                    }
                    output_ids[O_RST]->update = 1;                    
                } break;
                case I_BRB0: {
                    p_BT[0] = 1;
                    output_ids[O_BRB0]->update = 1;
                }break;
                case I_BRB1: {
                    p_BT[1] = 1;
                    output_ids[O_BRB1]->update = 1;
                }
                case I_BRB2: {
                    p_BT[2] = 1;
                    output_ids[O_BRB2]->update =1;
                }
                case I_BRC0: {
                    p_BT[3] = 1;
                    output_ids[O_BRC0]->update = 1;
                }break;
                case I_BRC1: {
                    p_BT[4] = 1;
                    output_ids[O_BRC1]->update = 1;
                } break;
            }
        }        
    }
}

void cboard_LASE1::EvKeyPress(uint key, uint mask){
    if (key=='1') {
        if (PICSimLab.GetMcuPwr() && pic_reset(&pic, -1)) {
            PICSimLab.SetMcuPwr(0);
            PICSimLab.SetMcuRst(1);
        }
        p_RST = 0;
        for (int i = 0; i < 4; i++){
            output_ids[O_RD0 + i]->value = pic.pins[19-1 + i].oavalue;
            output_ids[O_RD0 + i]->update = 1;
        }
        for (int i = 0; i < 4; i++){
            output_ids[O_RD4 + i]->value = pic.pins[27-1 + i].oavalue;
            output_ids[O_RD4 + i]->update = 1;
        }
        output_ids[O_RST]->update = 1;        
    }
    if (key=='2') {
        p_BT[3] = 0;
        output_ids[O_BRC0]->update=1;        
    }
    if (key=='3') {
        p_BT[4] = 0;
        output_ids[O_BRC1]->update=1;        
    }
    if (key=='4') {
        p_BT[0] = 0;
        output_ids[O_BRB0]->update=1;        
    }
    if (key=='5') {
        p_BT[1] = 0;
        output_ids[O_BRB1]->update=1;        
    }
    if (key=='6') {
        p_BT[2] = 0;
        output_ids[O_BRB2]->update=1;
    }
}
void cboard_LASE1::EvKeyRelease(uint key, uint mask){
    if (key=='1') {
        if (PICSimLab.GetMcuRst()) {
            PICSimLab.SetMcuPwr(1);
            PICSimLab.SetMcuRst(0);
            if (pic_reset(&pic, -1))
                Reset();                        
        }
        p_RST = 1;
        for (int i = 0; i < 4; i++){
            output_ids[O_RD0 + i]->value = pic.pins[19-1 + i].oavalue;
            output_ids[O_RD0 + i]->update = 1;
        }
        for (int i = 0; i < 4; i++){
            output_ids[O_RD4 + i]->value = pic.pins[27-1 + i].oavalue;
            output_ids[O_RD4 + i]->update = 1;
        }
        output_ids[O_RST]->update = 1;        
    }
    if (key=='2') {
        p_BT[3] = 1;
        output_ids[O_BRC0]->update=1;        
    }
    if (key=='3') {
        p_BT[4] = 1;
        output_ids[O_BRC1]->update=1;        
    }
    if (key=='4') {
        p_BT[0] = 1;
        output_ids[O_BRB0]->update=1;        
    }
    if (key=='5') {
        p_BT[1] = 1;
        output_ids[O_BRB1]->update=1;        
    }
    if (key=='6') {
        p_BT[2] = 1;
        output_ids[O_BRB2]->update=1;
    }
}

unsigned short cboard_LASE1::GetInputId(char* name){
    if (strcmp(name, "PG_ICSP") == 0)
        return I_ICSP;
    if (strcmp(name, "SW_PWR") == 0)
        return I_PWR;
    if (strcmp(name, "PB_INT0") == 0)
        return I_BRB0;
    if (strcmp(name, "PB_INT1") == 0)
        return I_BRB1;
    if (strcmp(name, "PB_INT2") == 0)
        return I_BRB2;    
    if (strcmp(name, "PB_TMR1") == 0)
        return I_BRC0;
    if (strcmp(name, "PB_RC1") == 0)
        return I_BRC1;
    if (strcmp(name, "PB_RST") == 0)
        return I_RST;
    if (strcmp(name, "DP_CH0") == 0)
        return I_SRB0;
    if (strcmp(name, "DP_CH1") == 0)
        return I_SRB1;
    if (strcmp(name, "DP_CH2") == 0)
        return I_SRB2;
    if (strcmp(name, "DP_CH3") == 0)
        return I_SRB3;
    if (strcmp(name, "DP_CH4") == 0)
        return I_SRB4;
    if (strcmp(name, "DP_CH5") == 0)
        return I_SRB5;
    if (strcmp(name, "DP_CH6") == 0)
        return I_SRB6;
    if (strcmp(name, "DP_CH7") == 0)
        return I_SRB7;
    if (strcmp(name, "DP_PGC") == 0)
        return I_SPGC;
    if (strcmp(name, "DP_PGD") == 0)
        return I_SPGD;
    if (strcmp(name, "DP_5V") == 0)
        return I_S5V;
    if (strcmp(name, "DP_VPP") == 0)
        return I_SVPP;
    printf("Error input '%s' don't have a valid id! \n", name);
    return INVALID_ID;
}

unsigned short cboard_LASE1::GetOutputId(char* name){
    if(strcmp(name, "IC_CPU")==0)
        return O_CPU;

    if (strcmp(name, "PB_INT0") == 0)
        return O_BRB0;
    if (strcmp(name, "PB_INT1") == 0)
        return O_BRB1;
    if (strcmp(name, "PB_INT2") == 0)
        return O_BRB2;
    if (strcmp(name, "PB_RST") == 0)
        return O_RST;
    if (strcmp(name, "PB_TMR1") == 0)
        return O_BRC0;
    if (strcmp(name, "PB_RC1") == 0)
        return O_BRC1;

    if (strcmp(name, "SS_1") == 0)
        return O_SS1;
    if (strcmp(name, "SS_2") == 0)
        return O_SS2;
    if (strcmp(name, "SS_3") == 0)
        return O_SS3;
    if (strcmp(name, "SS_4") == 0)
        return O_SS4;

    if (strcmp(name, "SS_A1") == 0)
        return O_A1;
    if (strcmp(name, "SS_B1") == 0)
        return O_B1;
    if (strcmp(name, "SS_C1") == 0)
        return O_C1;
    if (strcmp(name, "SS_D1") == 0)
        return O_D1;
    if (strcmp(name, "SS_E1") == 0)
        return O_E1;
    if (strcmp(name, "SS_F1") == 0)
        return O_F1;
    if (strcmp(name, "SS_G1") == 0)
        return O_G1;
    if (strcmp(name, "SS_P1") == 0)
        return O_P1;

    if (strcmp(name, "SS_A2") == 0)
        return O_A2;
    if (strcmp(name, "SS_B2") == 0)
        return O_B2;
    if (strcmp(name, "SS_C2") == 0)
        return O_C2;
    if (strcmp(name, "SS_D2") == 0)
        return O_D2;
    if (strcmp(name, "SS_E2") == 0)
        return O_E2;
    if (strcmp(name, "SS_F2") == 0)
        return O_F2;
    if (strcmp(name, "SS_G2") == 0)
        return O_G2;
    if (strcmp(name, "SS_P2") == 0)
        return O_P2;
    
    if (strcmp(name, "SS_A3") == 0)
        return O_A3;
    if (strcmp(name, "SS_B3") == 0)
        return O_B3;
    if (strcmp(name, "SS_C3") == 0)
        return O_C3;
    if (strcmp(name, "SS_D3") == 0)
        return O_D3;
    if (strcmp(name, "SS_E3") == 0)
        return O_E3;
    if (strcmp(name, "SS_F3") == 0)
        return O_F3;
    if (strcmp(name, "SS_G3") == 0)
        return O_G3;
    if (strcmp(name, "SS_P3") == 0)
        return O_P3;

    if (strcmp(name, "SS_A4") == 0)
        return O_A4;
    if (strcmp(name, "SS_B4") == 0)
        return O_B4;
    if (strcmp(name, "SS_C4") == 0)
        return O_C4;
    if (strcmp(name, "SS_D4") == 0)
        return O_D4;
    if (strcmp(name, "SS_E4") == 0)
        return O_E4;
    if (strcmp(name, "SS_F4") == 0)
        return O_F4;
    if (strcmp(name, "SS_G4") == 0)
        return O_G4;
    if (strcmp(name, "SS_P4") == 0)
        return O_P4;

    if (strcmp(name, "LD_LPWR") == 0)
        return O_LPWR;    
    if (strcmp(name, "LD_LICSP") == 0)
        return O_LICSP;
    if (strcmp(name, "LD_RD0") == 0)
        return O_RD0;
    if (strcmp(name, "LD_RD1") == 0)
        return O_RD1;
    if (strcmp(name, "LD_RD2") == 0)
        return O_RD2;
    if (strcmp(name, "LD_RD3") == 0)
        return O_RD3;
    if (strcmp(name, "LD_RD4") == 0)
        return O_RD4;
    if (strcmp(name, "LD_RD5") == 0)
        return O_RD5;
    if (strcmp(name, "LD_RD6") == 0)
        return O_RD6;
    if (strcmp(name, "LD_RD7") == 0)
        return O_RD7;

    if(strcmp(name, "BC1")==0)
        return O_BC1;
    if(strcmp(name, "LD_R1")==0)
        return O_BCR1;
    if(strcmp(name, "LD_G1")==0)
        return O_BCG1;    
    if(strcmp(name, "BC2")==0)
        return O_BC2;
    if(strcmp(name, "LD_R2")==0)
        return O_BCR2;
    if(strcmp(name, "LD_G2")==0)
        return O_BCG2;

    if(strcmp(name, "LD_LAMP")==0)
        return O_LAMP;

    if (strcmp(name, "DP_CH0") == 0)
        return O_SRB0;
    if (strcmp(name, "DP_CH1") == 0)
        return O_SRB1;
    if (strcmp(name, "DP_CH2") == 0)
        return O_SRB2;
    if (strcmp(name, "DP_CH3") == 0)
        return O_SRB3;
    if (strcmp(name, "DP_CH4") == 0)
        return O_SRB4;
    if (strcmp(name, "DP_CH5") == 0)
        return O_SRB5;
    if (strcmp(name, "DP_CH6") == 0)
        return O_SRB6;
    if (strcmp(name, "DP_CH7") == 0)
        return O_SRB7;
    if (strcmp(name, "DP_PGC") == 0)
        return O_SPGC;
    if (strcmp(name, "DP_PGD") == 0)
        return O_SPGD;
    if (strcmp(name, "DP_5V") == 0)
        return O_S5V;
    if (strcmp(name, "DP_VPP") == 0)
        return O_SVPP;

    if (strcmp(name, "RL1") == 0)
        return O_RL1;
    if (strcmp(name, "RL2") == 0)
        return O_RL2;
    if (strcmp(name, "RL3") == 0)
        return O_RL3;
    if (strcmp(name, "RL4") == 0)
        return O_RL4;

    printf("Error output '%s' don't have a valid id! \n", name);
    return INVALID_ID;
}

void cboard_LASE1::WritePreferences(void) {
    char line[100];
    PICSimLab.SavePrefs(lxT("LASE1_proc"), Proc);

    line[0] = 0;
    for (int i = 0; i < 8; i++)
        sprintf(line + i, "%i", dip[i]);

    PICSimLab.SavePrefs(lxT("LASE1_dip"), line);
    PICSimLab.SavePrefs(lxT("LASE1_clock"), lxString().Format("%2.1f", PICSimLab.GetClock()));
}

void cboard_LASE1::ReadPreferences(char* name, char* value) {
    if (!strcmp(name, "LASE1_proc")) {
        Proc = value;
    }

    int i;
    if (!strcmp(name, "LASE1_dip")) {
        for (i = 0; i < 8; i++) {
            if (value[i] == '0')
                dip[i] = 0;
            else
                dip[i] = 1;
        }
    }

    if (!strcmp(name, "LASE1_clock")) {
        PICSimLab.SetClock(atof(value));
    }
}

board_init(BOARD_LASE1_Name, cboard_LASE1);
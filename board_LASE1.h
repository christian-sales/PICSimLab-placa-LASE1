#ifndef BOARD_LASE1_H
#define BOARD_LASE1_H

#include <lxrad.h> // ferramentas para desenvolvimento de aplicativos
#include "../devices/swbounce.h" // controle do efeito bounce em switches
#include "bsim_picsim.h" // funções de simulação e debug

#define BOARD_LASE1_Name "LASE1" // define o nome da placa

class cboard_LASE1 : public bsim_picsim{
private:
    // indica o estado do buzzer
    int sound_on;

    // indica o estado dos relés
    unsigned char relay[4];
    // guarda a imagem dos relés
    lxBitmap* relay_i[2];

    // indica o estado dos botões
    unsigned char p_BT[5];

    //indica o estado das chaves
    int dip[12];

    unsigned int lm1[40]; // luminosidade média do display
    unsigned int lm2[40]; // luminosidade média do display
    unsigned int lm3[40]; // luminosidade média do display
    unsigned int lm4[40]; // luminosidade média do display

    //LAMP
    CGauge* gauge1; // medidor
    CLabel* label1; // rótulo 

    lxaudio buzzer; // objeto buzzer    

    lxColor color1; // desenho
    lxColor color2; // dos LEDs

    lxFont font;
    SWBounce_t bounce;

    void RegisterRemoteControl(void) override;

public:
    lxString GetName(void) override {return lxT(BOARD_LASE1_Name);}; // retorna o nome da placa
    lxString GetAboutInfo(void) override {return lxT(":\nChristian Sales\nAchilles Martins Silva\nJuliano Coêlho Miranda\n<juliano.coelhomiranda@cefetmg.br>");}
    // retorna informações e da placa
    cboard_LASE1(void); // construtor
    ~cboard_LASE1(void); // destrutor
    void Draw(CDraw* draw) override; // chamada a cada 100ms para redesenhar a placa
    void Run_CPU(void) override; // simulação
    lxString GetSupportedDevices(void) override {return lxT("PIC18F4550");}; // retorna uma lista com os microcontroladores suportados
    void Reset(void) override; // reseta a placa
    void EvMouseButtonPress(uint button, uint x, uint y, uint state) override;   // eventos
    void EvMouseButtonRelease(uint button, uint x, uint y, uint state) override; // de mouse
    void EvKeyPress(uint key, uint mask) override;   // eventos 
    void EvKeyRelease(uint key, uint mask) override; // de teclado
    void WritePreferences(void) override;                  // salvar e carregar 
    void ReadPreferences(char* name, char* value)override; // preferências da placa
    unsigned short GetInputId(char* name) override; // retorna o id dos nomes das entradas no map
    unsigned short GetOutputId(char* name) override; // retorna o id dos nomes das saídas no map
    int GetDefaultClock(void) override {return 4;}; // retorna o clock padrão
    void SetScale(double scale) override; // controla a escala (para as imagens)
};

#endif /* BOARD_LASE1_H*/
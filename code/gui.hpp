#ifndef GUI_HPP
#define GUI_HPP

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Choice.H>
#include <string>
#include <vector>

#include "image_box.hpp"
#include "stego_engine.hpp"

class MainWindow : public Fl_Double_Window {
public:
    MainWindow(int w, int h, const char *title = nullptr);
    ~MainWindow() override;

private:
    void setupTheme();
    void buildTabs();
    void buildEncodeTab(int x, int y, int w, int h);
    void buildDecodeTab(int x, int y, int w, int h);
    void buildAnalysisTab(int x, int y, int w, int h);

    // Callbacks helpers
    void onSelectCarrierImage();
    void onSelectPayloadFile();
    void onPayloadModeChanged();
    void onUpdateEncodeStats();
    void onEncodeAndSave();

    void onSelectStegoImage();
    void onDecode();
    void onCopyDecodedText();
    void onSaveDecodedFile();

    void onSelectAnalysisImage();
    void onAnalysisModeChanged(int mode);

    // Static callback routers
    static void cbSelectCarrierImage(Fl_Widget*, void *v) { static_cast<MainWindow*>(v)->onSelectCarrierImage(); }
    static void cbSelectPayloadFile(Fl_Widget*, void *v) { static_cast<MainWindow*>(v)->onSelectPayloadFile(); }
    static void cbPayloadModeChanged(Fl_Widget*, void *v) { static_cast<MainWindow*>(v)->onPayloadModeChanged(); }
    static void cbTextBufferModified(int, int, int, int, const char*, void *v) { static_cast<MainWindow*>(v)->onUpdateEncodeStats(); }
    static void cbEncodeAndSave(Fl_Widget*, void *v) { static_cast<MainWindow*>(v)->onEncodeAndSave(); }

    static void cbSelectStegoImage(Fl_Widget*, void *v) { static_cast<MainWindow*>(v)->onSelectStegoImage(); }
    static void cbDecode(Fl_Widget*, void *v) { static_cast<MainWindow*>(v)->onDecode(); }
    static void cbCopyDecodedText(Fl_Widget*, void *v) { static_cast<MainWindow*>(v)->onCopyDecodedText(); }
    static void cbSaveDecodedFile(Fl_Widget*, void *v) { static_cast<MainWindow*>(v)->onSaveDecodedFile(); }

    static void cbSelectAnalysisImage(Fl_Widget*, void *v) { static_cast<MainWindow*>(v)->onSelectAnalysisImage(); }
    static void cbAnalysisMode0(Fl_Widget*, void *v) { static_cast<MainWindow*>(v)->onAnalysisModeChanged(0); }
    static void cbAnalysisMode1(Fl_Widget*, void *v) { static_cast<MainWindow*>(v)->onAnalysisModeChanged(1); }
    static void cbAnalysisMode2(Fl_Widget*, void *v) { static_cast<MainWindow*>(v)->onAnalysisModeChanged(2); }

    // Tab 1 (Encode) widgets
    ImageBox *m_encodeImgBox = nullptr;
    Fl_Box *m_encodeInfoLabel = nullptr;
    Fl_Choice *m_encodeModeChoice = nullptr;
    Fl_Text_Editor *m_encodeTextEditor = nullptr;
    Fl_Text_Buffer *m_encodeTextBuf = nullptr;
    Fl_Button *m_encodeFileBtn = nullptr;
    Fl_Box *m_encodeFileLabel = nullptr;
    Fl_Secret_Input *m_encodePasswordInput = nullptr;
    Fl_Progress *m_encodeProgress = nullptr;
    Fl_Box *m_encodeCapacityLabel = nullptr;
    Fl_Button *m_encodeSaveBtn = nullptr;
    Fl_Box *m_encodeStatusBox = nullptr;

    std::string m_carrierFilePath;
    std::string m_payloadFilePath;
    std::vector<uint8_t> m_payloadFileData;

    // Tab 2 (Decode) widgets
    ImageBox *m_decodeImgBox = nullptr;
    Fl_Box *m_decodeInfoLabel = nullptr;
    Fl_Secret_Input *m_decodePasswordInput = nullptr;
    Fl_Button *m_decodeActionBtn = nullptr;
    Fl_Box *m_decodeResultCard = nullptr;
    Fl_Text_Display *m_decodeTextDisplay = nullptr;
    Fl_Text_Buffer *m_decodeTextBuf = nullptr;
    Fl_Button *m_decodeCopyBtn = nullptr;
    Fl_Button *m_decodeSaveFileBtn = nullptr;
    Fl_Box *m_decodeStatusBox = nullptr;

    std::string m_stegoFilePath;
    StegoPayload m_extractedPayload;

    // Tab 3 (Analysis) widgets
    ImageBox *m_analysisImgBox = nullptr;
    Fl_Box *m_analysisInfoLabel = nullptr;
    Fl_Button *m_analysisBtnOrig = nullptr;
    Fl_Button *m_analysisBtnLsb0 = nullptr;
    Fl_Button *m_analysisBtnLsb1 = nullptr;
    Fl_Box *m_analysisDescBox = nullptr;

    std::vector<uint8_t> m_analysisRawOrig;
    int m_analysisW = 0;
    int m_analysisH = 0;
    int m_analysisCh = 0;
};

#endif // GUI_HPP

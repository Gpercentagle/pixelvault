#include "gui.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <iostream>

namespace {

// Colors
const Fl_Color COLOR_BG       = fl_rgb_color(30, 30, 46);   // Dark slate
const Fl_Color COLOR_PANEL    = fl_rgb_color(24, 24, 37);   // Darker panel
const Fl_Color COLOR_CARD     = fl_rgb_color(36, 36, 54);   // Slightly lighter card
const Fl_Color COLOR_INPUT    = fl_rgb_color(49, 50, 68);   // Input background
const Fl_Color COLOR_BORDER   = fl_rgb_color(69, 71, 90);   // Subtle border
const Fl_Color COLOR_TEXT     = fl_rgb_color(205, 214, 244);// Soft white
const Fl_Color COLOR_MUTED    = fl_rgb_color(166, 173, 200);// Grey text
const Fl_Color COLOR_ACCENT   = fl_rgb_color(137, 180, 250);// Blue accent
const Fl_Color COLOR_SUCCESS  = fl_rgb_color(166, 227, 161);// Green accent
const Fl_Color COLOR_ERROR    = fl_rgb_color(243, 139, 168);// Red/Coral

std::string formatBytes(size_t bytes) {
    if (bytes < 1024) return std::to_string(bytes) + " Б";
    double kb = bytes / 1024.0;
    if (kb < 1024.0) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << kb << " КБ";
        return ss.str();
    }
    double mb = kb / 1024.0;
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << mb << " МБ";
    return ss.str();
}

std::vector<uint8_t> readFileBytes(const std::string &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(size);
    if (size > 0) {
        file.read(reinterpret_cast<char*>(buffer.data()), size);
    }
    return buffer;
}

std::string getFilenameOnly(const std::string &path) {
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) return path.substr(pos + 1);
    return path;
}

} // anonymous namespace

MainWindow::MainWindow(int w, int h, const char *title)
    : Fl_Double_Window(w, h, title) {
    setupTheme();
    buildTabs();
    end();
}

MainWindow::~MainWindow() {
    delete m_encodeTextBuf;
    delete m_decodeTextBuf;
}

void MainWindow::setupTheme() {
    Fl::background(30, 30, 46);
    Fl::background2(49, 50, 68);
    Fl::foreground(205, 214, 244);
    color(COLOR_BG);
}

void MainWindow::buildTabs() {
    Fl_Tabs *tabs = new Fl_Tabs(10, 10, w() - 20, h() - 20);
    tabs->color(COLOR_PANEL);
    tabs->labelcolor(COLOR_TEXT);
    tabs->selection_color(COLOR_CARD);

    // Tab 1: Encode
    buildEncodeTab(10, 40, w() - 20, h() - 50);

    // Tab 2: Decode
    buildDecodeTab(10, 40, w() - 20, h() - 50);

    // Tab 3: Stego Analysis
    buildAnalysisTab(10, 40, w() - 20, h() - 50);

    tabs->end();
}

void MainWindow::buildEncodeTab(int x, int y, int w, int h) {
    Fl_Group *grp = new Fl_Group(x, y, w, h, "  Скрыть данные (Encode)  ");
    grp->color(COLOR_BG);
    grp->labelsize(14);
    grp->labelcolor(COLOR_TEXT);

    int leftW = 430;
    int rightX = x + leftW + 20;
    int rightW = w - leftW - 30;

    // --- Left column: Image & Capacity ---
    Fl_Button *btnSelectImg = new Fl_Button(x + 10, y + 15, leftW, 36, " Выбрать изображение (PNG, JPG, BMP)...");
    btnSelectImg->color(COLOR_INPUT);
    btnSelectImg->labelcolor(COLOR_ACCENT);
    btnSelectImg->labelsize(13);
    btnSelectImg->callback(cbSelectCarrierImage, this);

    m_encodeImgBox = new ImageBox(x + 10, y + 60, leftW, 380);
    m_encodeImgBox->setPlaceholder("Нажмите кнопку выше для выбора изображения-носителя");

    m_encodeInfoLabel = new Fl_Box(x + 10, y + 450, leftW, 30, "Изображение не загружено");
    m_encodeInfoLabel->align(FL_ALIGN_CENTER);
    m_encodeInfoLabel->labelcolor(COLOR_MUTED);
    m_encodeInfoLabel->labelsize(12);

    m_encodeCapacityLabel = new Fl_Box(x + 10, y + 480, leftW, 20, "Емкость: 0 Б");
    m_encodeCapacityLabel->align(FL_ALIGN_CENTER);
    m_encodeCapacityLabel->labelcolor(COLOR_TEXT);
    m_encodeCapacityLabel->labelsize(13);

    m_encodeProgress = new Fl_Progress(x + 10, y + 510, leftW, 18);
    m_encodeProgress->color(COLOR_INPUT);
    m_encodeProgress->selection_color(COLOR_SUCCESS);
    m_encodeProgress->minimum(0);
    m_encodeProgress->maximum(100);
    m_encodeProgress->value(0);

    // --- Right column: Payload configuration ---
    Fl_Box *lblPayload = new Fl_Box(rightX, y + 15, 120, 30, "Тип данных:");
    lblPayload->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    lblPayload->labelcolor(COLOR_TEXT);
    lblPayload->labelsize(13);

    m_encodeModeChoice = new Fl_Choice(rightX + 110, y + 15, 180, 30);
    m_encodeModeChoice->add("Текстовое сообщение");
    m_encodeModeChoice->add("Файл (любой формат)");
    m_encodeModeChoice->value(0);
    m_encodeModeChoice->color(COLOR_INPUT);
    m_encodeModeChoice->labelcolor(COLOR_TEXT);
    m_encodeModeChoice->callback(cbPayloadModeChanged, this);

    // Text Editor
    m_encodeTextBuf = new Fl_Text_Buffer();
    m_encodeTextBuf->text("Секретное сообщение для внедрения в картинку.\nPixelVault надежно спрячет его в младших битах пикселей!");
    m_encodeTextBuf->add_modify_callback(cbTextBufferModified, this);

    m_encodeTextEditor = new Fl_Text_Editor(rightX, y + 55, rightW, 220);
    m_encodeTextEditor->buffer(m_encodeTextBuf);
    m_encodeTextEditor->color(COLOR_PANEL);
    m_encodeTextEditor->textcolor(COLOR_TEXT);
    m_encodeTextEditor->textsize(13);

    // File Selector widgets (initially hidden)
    m_encodeFileBtn = new Fl_Button(rightX, y + 55, 180, 36, " Выбрать файл...");
    m_encodeFileBtn->color(COLOR_INPUT);
    m_encodeFileBtn->labelcolor(COLOR_ACCENT);
    m_encodeFileBtn->callback(cbSelectPayloadFile, this);
    m_encodeFileBtn->hide();

    m_encodeFileLabel = new Fl_Box(rightX + 190, y + 55, rightW - 190, 36, "Файл не выбран");
    m_encodeFileLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    m_encodeFileLabel->labelcolor(COLOR_MUTED);
    m_encodeFileLabel->labelsize(12);
    m_encodeFileLabel->hide();

    // Password input
    Fl_Box *lblPwd = new Fl_Box(rightX, y + 290, rightW, 20, "Пароль шифрования (SHA-256 + Stream Cipher, опционально):");
    lblPwd->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    lblPwd->labelcolor(COLOR_MUTED);
    lblPwd->labelsize(12);

    m_encodePasswordInput = new Fl_Secret_Input(rightX, y + 315, rightW, 36);
    m_encodePasswordInput->color(COLOR_INPUT);
    m_encodePasswordInput->textcolor(COLOR_TEXT);
    m_encodePasswordInput->textsize(14);

    // Save action button
    m_encodeSaveBtn = new Fl_Button(rightX, y + 375, rightW, 46, " Зашифровать и Сохранить PNG...");
    m_encodeSaveBtn->color(COLOR_ACCENT);
    m_encodeSaveBtn->labelcolor(fl_rgb_color(17, 17, 27));
    m_encodeSaveBtn->labelsize(15);
    m_encodeSaveBtn->labelfont(FL_HELVETICA_BOLD);
    m_encodeSaveBtn->callback(cbEncodeAndSave, this);

    // Status box
    m_encodeStatusBox = new Fl_Box(rightX, y + 435, rightW, 90, "Готов к работе. Выберите изображение для старта.");
    m_encodeStatusBox->box(FL_FLAT_BOX);
    m_encodeStatusBox->color(COLOR_CARD);
    m_encodeStatusBox->labelcolor(COLOR_MUTED);
    m_encodeStatusBox->labelsize(13);
    m_encodeStatusBox->align(FL_ALIGN_INSIDE | FL_ALIGN_WRAP | FL_ALIGN_TOP_LEFT);

    grp->end();
}

void MainWindow::buildDecodeTab(int x, int y, int w, int h) {
    Fl_Group *grp = new Fl_Group(x, y, w, h, "  Извлечь данные (Decode)  ");
    grp->color(COLOR_BG);
    grp->labelsize(14);
    grp->labelcolor(COLOR_TEXT);

    int leftW = 430;
    int rightX = x + leftW + 20;
    int rightW = w - leftW - 30;

    // Left Column
    Fl_Button *btnSelectStego = new Fl_Button(x + 10, y + 15, leftW, 36, " Выбрать Stego-изображение (PNG)...");
    btnSelectStego->color(COLOR_INPUT);
    btnSelectStego->labelcolor(COLOR_ACCENT);
    btnSelectStego->labelsize(13);
    btnSelectStego->callback(cbSelectStegoImage, this);

    m_decodeImgBox = new ImageBox(x + 10, y + 60, leftW, 380);
    m_decodeImgBox->setPlaceholder("Выберите изображение, содержащее скрытые данные");

    m_decodeInfoLabel = new Fl_Box(x + 10, y + 450, leftW, 30, "Изображение не загружено");
    m_decodeInfoLabel->align(FL_ALIGN_CENTER);
    m_decodeInfoLabel->labelcolor(COLOR_MUTED);
    m_decodeInfoLabel->labelsize(12);

    // Right Column
    Fl_Box *lblPwd = new Fl_Box(rightX, y + 15, rightW, 20, "Пароль расшифровки (если был задан при сохранении):");
    lblPwd->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    lblPwd->labelcolor(COLOR_MUTED);
    lblPwd->labelsize(12);

    m_decodePasswordInput = new Fl_Secret_Input(rightX, y + 40, rightW, 36);
    m_decodePasswordInput->color(COLOR_INPUT);
    m_decodePasswordInput->textcolor(COLOR_TEXT);
    m_decodePasswordInput->textsize(14);

    m_decodeActionBtn = new Fl_Button(rightX, y + 85, rightW, 40, " Извлечь скрытые данные");
    m_decodeActionBtn->color(COLOR_SUCCESS);
    m_decodeActionBtn->labelcolor(fl_rgb_color(17, 17, 27));
    m_decodeActionBtn->labelsize(14);
    m_decodeActionBtn->labelfont(FL_HELVETICA_BOLD);
    m_decodeActionBtn->callback(cbDecode, this);

    // Result card
    m_decodeResultCard = new Fl_Box(rightX, y + 135, rightW, 55, "Контейнер: не проанализирован");
    m_decodeResultCard->box(FL_FLAT_BOX);
    m_decodeResultCard->color(COLOR_CARD);
    m_decodeResultCard->labelcolor(COLOR_TEXT);
    m_decodeResultCard->labelsize(13);
    m_decodeResultCard->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

    // Text Display
    m_decodeTextBuf = new Fl_Text_Buffer();
    m_decodeTextDisplay = new Fl_Text_Display(rightX, y + 200, rightW, 230);
    m_decodeTextDisplay->buffer(m_decodeTextBuf);
    m_decodeTextDisplay->color(COLOR_PANEL);
    m_decodeTextDisplay->textcolor(COLOR_TEXT);
    m_decodeTextDisplay->textsize(13);

    // Output Action buttons
    m_decodeCopyBtn = new Fl_Button(rightX, y + 440, (rightW - 10) / 2, 36, " Скопировать текст");
    m_decodeCopyBtn->color(COLOR_INPUT);
    m_decodeCopyBtn->labelcolor(COLOR_TEXT);
    m_decodeCopyBtn->deactivate();
    m_decodeCopyBtn->callback(cbCopyDecodedText, this);

    m_decodeSaveFileBtn = new Fl_Button(rightX + (rightW - 10) / 2 + 10, y + 440, (rightW - 10) / 2, 36, " Сохранить файл на диск...");
    m_decodeSaveFileBtn->color(COLOR_INPUT);
    m_decodeSaveFileBtn->labelcolor(COLOR_TEXT);
    m_decodeSaveFileBtn->deactivate();
    m_decodeSaveFileBtn->callback(cbSaveDecodedFile, this);

    // Status box
    m_decodeStatusBox = new Fl_Box(rightX, y + 485, rightW, 45, "");
    m_decodeStatusBox->box(FL_FLAT_BOX);
    m_decodeStatusBox->color(COLOR_PANEL);
    m_decodeStatusBox->labelcolor(COLOR_MUTED);
    m_decodeStatusBox->labelsize(12);
    m_decodeStatusBox->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

    grp->end();
}

void MainWindow::buildAnalysisTab(int x, int y, int w, int h) {
    Fl_Group *grp = new Fl_Group(x, y, w, h, "  LSB Анализатор (Стеганоанализ)  ");
    grp->color(COLOR_BG);
    grp->labelsize(14);
    grp->labelcolor(COLOR_TEXT);

    int leftW = 460;
    int rightX = x + leftW + 20;
    int rightW = w - leftW - 30;

    Fl_Button *btnSelect = new Fl_Button(x + 10, y + 15, leftW, 36, " Загрузить картинку для анализа...");
    btnSelect->color(COLOR_INPUT);
    btnSelect->labelcolor(COLOR_ACCENT);
    btnSelect->callback(cbSelectAnalysisImage, this);

    m_analysisImgBox = new ImageBox(x + 10, y + 60, leftW, 420);
    m_analysisImgBox->setPlaceholder("Загрузите изображение для исследования младших бит (LSB)");

    m_analysisInfoLabel = new Fl_Box(x + 10, y + 490, leftW, 30, "Изображение не загружено");
    m_analysisInfoLabel->align(FL_ALIGN_CENTER);
    m_analysisInfoLabel->labelcolor(COLOR_MUTED);
    m_analysisInfoLabel->labelsize(12);

    // Right side: controls & explanation
    Fl_Box *lblModes = new Fl_Box(rightX, y + 15, rightW, 25, "Режимы визуализации срезов:");
    lblModes->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    lblModes->labelcolor(COLOR_TEXT);
    lblModes->labelsize(14);
    lblModes->labelfont(FL_HELVETICA_BOLD);

    m_analysisBtnOrig = new Fl_Button(rightX, y + 45, rightW, 36, "Оригинальное изображение");
    m_analysisBtnOrig->color(COLOR_INPUT);
    m_analysisBtnOrig->labelcolor(COLOR_TEXT);
    m_analysisBtnOrig->callback(cbAnalysisMode0, this);

    m_analysisBtnLsb0 = new Fl_Button(rightX, y + 90, rightW, 36, "LSB Срез 0 (Младший бит - Высокий шум)");
    m_analysisBtnLsb0->color(COLOR_INPUT);
    m_analysisBtnLsb0->labelcolor(COLOR_ACCENT);
    m_analysisBtnLsb0->callback(cbAnalysisMode1, this);

    m_analysisBtnLsb1 = new Fl_Button(rightX, y + 135, rightW, 36, "LSB Срез 1 (Второй бит)");
    m_analysisBtnLsb1->color(COLOR_INPUT);
    m_analysisBtnLsb1->labelcolor(COLOR_TEXT);
    m_analysisBtnLsb1->callback(cbAnalysisMode2, this);

    // Educational / forensic explanation
    std::string desc = 
        "Как работает LSB стеганография?\n\n"
        "1. Каждый пиксель состоит из компонент R, G, B (по 8 бит от 0 до 255).\n"
        "2. Младший бит (Bit 0) отвечает лишь за +/-1 яркости, что абсолютно незаметно человеческому глазу.\n"
        "3. PixelVault заменяет младшие биты байтами зашифрованного сообщения или файла.\n\n"
        "Стеганоанализ:\n"
        "В натуральных фото срез Bit 0 выглядит как хаотичный природный шум матрицы.\n"
        "Если в область внедрены зашифрованные данные, энтропия становится равномерной псевдослучайной последовательностью.";

    m_analysisDescBox = new Fl_Box(rightX, y + 190, rightW, 330);
    m_analysisDescBox->box(FL_FLAT_BOX);
    m_analysisDescBox->color(COLOR_CARD);
    m_analysisDescBox->labelcolor(COLOR_MUTED);
    m_analysisDescBox->labelsize(12);
    m_analysisDescBox->copy_label(desc.c_str());
    m_analysisDescBox->align(FL_ALIGN_INSIDE | FL_ALIGN_WRAP | FL_ALIGN_TOP_LEFT);

    grp->end();
}

// ----------------- CALLBACKS & LOGIC -----------------

void MainWindow::onSelectCarrierImage() {
    Fl_Native_File_Chooser fnfc;
    fnfc.title("Выберите изображение-носитель");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fnfc.filter("Изображения\t*.{png,jpg,jpeg,bmp}");

    if (fnfc.show() == 0) {
        std::string path = fnfc.filename();
        Fl_Shared_Image *img = Fl_Shared_Image::get(path.c_str());
        if (!img || img->w() == 0) {
            fl_alert("Не удалось загрузить выбранное изображение.");
            return;
        }

        m_carrierFilePath = path;
        int w = img->w();
        int h = img->h();
        int d = img->d();

        m_encodeImgBox->setImage((const uint8_t*)img->data()[0], w, h, d);

        size_t cap = StegoEngine::getCapacityBytes(w, h, d);
        std::ostringstream ss;
        ss << getFilenameOnly(path) << " (" << w << "x" << h << ", " << d << " каналов)";
        m_encodeInfoLabel->copy_label(ss.str().c_str());

        std::string capStr = "Максимальная емкость: " + formatBytes(cap);
        m_encodeCapacityLabel->copy_label(capStr.c_str());

        onUpdateEncodeStats();
        m_encodeStatusBox->copy_label("Изображение готово. Введите текст или выберите файл для скрытия.");
        m_encodeStatusBox->labelcolor(COLOR_TEXT);
    }
}

void MainWindow::onPayloadModeChanged() {
    int mode = m_encodeModeChoice->value();
    if (mode == 0) { // Text mode
        m_encodeTextEditor->show();
        m_encodeFileBtn->hide();
        m_encodeFileLabel->hide();
    } else { // File mode
        m_encodeTextEditor->hide();
        m_encodeFileBtn->show();
        m_encodeFileLabel->show();
    }
    onUpdateEncodeStats();
}

void MainWindow::onSelectPayloadFile() {
    Fl_Native_File_Chooser fnfc;
    fnfc.title("Выберите файл для внедрения");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fnfc.filter("Все файлы\t*.*");

    if (fnfc.show() == 0) {
        m_payloadFilePath = fnfc.filename();
        m_payloadFileData = readFileBytes(m_payloadFilePath);
        std::string fname = getFilenameOnly(m_payloadFilePath);

        std::string labelText = fname + " (" + formatBytes(m_payloadFileData.size()) + ")";
        m_encodeFileLabel->copy_label(labelText.c_str());
        onUpdateEncodeStats();
    }
}

void MainWindow::onUpdateEncodeStats() {
    if (!m_encodeImgBox->hasImage()) {
        m_encodeProgress->value(0);
        return;
    }

    size_t cap = StegoEngine::getCapacityBytes(
        m_encodeImgBox->getImageWidth(),
        m_encodeImgBox->getImageHeight(),
        m_encodeImgBox->getImageChannels()
    );

    size_t payloadSize = 0;
    if (m_encodeModeChoice->value() == 0) {
        if (m_encodeTextBuf) {
            payloadSize = m_encodeTextBuf->length();
        }
    } else {
        payloadSize = m_payloadFileData.size();
    }

    if (cap > 0) {
        double pct = (static_cast<double>(payloadSize) / cap) * 100.0;
        if (pct > 100.0) pct = 100.0;
        m_encodeProgress->value(pct);

        std::ostringstream ss;
        ss << "Занято: " << formatBytes(payloadSize) << " из " << formatBytes(cap)
           << " (" << std::fixed << std::setprecision(1) << pct << "%)";
        m_encodeCapacityLabel->copy_label(ss.str().c_str());

        if (payloadSize > cap) {
            m_encodeProgress->selection_color(COLOR_ERROR);
            m_encodeStatusBox->copy_label("Внимание: Размер данных превышает емкость изображения!");
            m_encodeStatusBox->labelcolor(COLOR_ERROR);
        } else {
            m_encodeProgress->selection_color(COLOR_SUCCESS);
        }
    }
}

void MainWindow::onEncodeAndSave() {
    if (!m_encodeImgBox->hasImage()) {
        fl_alert("Пожалуйста, сначала выберите исходное изображение.");
        return;
    }

    StegoPayload payload;
    int mode = m_encodeModeChoice->value();
    if (mode == 0) {
        payload.isFile = false;
        payload.filename = "";
        char *text = m_encodeTextBuf->text();
        if (text) {
            payload.data.assign(reinterpret_cast<uint8_t*>(text),
                                reinterpret_cast<uint8_t*>(text) + std::strlen(text));
            free(text);
        }
    } else {
        if (m_payloadFileData.empty()) {
            fl_alert("Пожалуйста, выберите файл для внедрения.");
            return;
        }
        payload.isFile = true;
        payload.filename = getFilenameOnly(m_payloadFilePath);
        payload.data = m_payloadFileData;
    }

    if (payload.data.empty()) {
        fl_alert("Данные для внедрения не могут быть пустыми.");
        return;
    }

    std::string password = m_encodePasswordInput->value() ? m_encodePasswordInput->value() : "";

    // Copy original pixels
    int w = m_encodeImgBox->getImageWidth();
    int h = m_encodeImgBox->getImageHeight();
    int ch = m_encodeImgBox->getImageChannels();
    size_t totalBytes = static_cast<size_t>(w) * h * ch;
    std::vector<uint8_t> stegoPixels(totalBytes);
    std::memcpy(stegoPixels.data(), m_encodeImgBox->getRawPixels(), totalBytes);

    std::string err;
    if (!StegoEngine::embed(stegoPixels.data(), w, h, ch, payload, password, err)) {
        fl_alert("Ошибка внедрения: %s", err.c_str());
        m_encodeStatusBox->copy_label(("Ошибка: " + err).c_str());
        m_encodeStatusBox->labelcolor(COLOR_ERROR);
        return;
    }

    // Choose destination save path
    Fl_Native_File_Chooser fnfc;
    fnfc.title("Сохранить зашифрованное изображение как PNG");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fnfc.filter("PNG Изображение (*.png)\t*.png");
    fnfc.preset_file("stego_image.png");

    if (fnfc.show() == 0) {
        std::string savePath = fnfc.filename();
        if (savePath.find(".png") == std::string::npos && savePath.find(".PNG") == std::string::npos) {
            savePath += ".png";
        }

        int res = fl_write_png(savePath.c_str(), stegoPixels.data(), w, h, ch);
        if (res == 0) {
            std::string msg = "Успешно сохранено!\nФайл: " + savePath +
                              "\nВнедрено: " + formatBytes(payload.data.size()) +
                              (password.empty() ? "\n(Без шифрования)" : "\n(Зашифровано SHA-256 + Stream Cipher)");
            fl_message("%s", msg.c_str());
            m_encodeStatusBox->copy_label(("Успешно сохранено в: " + getFilenameOnly(savePath)).c_str());
            m_encodeStatusBox->labelcolor(COLOR_SUCCESS);
        } else {
            fl_alert("Ошибка при сохранении PNG файла на диск.");
            m_encodeStatusBox->copy_label("Ошибка записи PNG файла.");
            m_encodeStatusBox->labelcolor(COLOR_ERROR);
        }
    }
}

void MainWindow::onSelectStegoImage() {
    Fl_Native_File_Chooser fnfc;
    fnfc.title("Выберите изображение со скрытыми данными");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fnfc.filter("PNG Изображение\t*.png");

    if (fnfc.show() == 0) {
        std::string path = fnfc.filename();
        Fl_Shared_Image *img = Fl_Shared_Image::get(path.c_str());
        if (!img || img->w() == 0) {
            fl_alert("Не удалось загрузить выбранное изображение.");
            return;
        }

        m_stegoFilePath = path;
        int w = img->w();
        int h = img->h();
        int d = img->d();
        m_decodeImgBox->setImage((const uint8_t*)img->data()[0], w, h, d);

        std::string info = getFilenameOnly(path) + " (" + std::to_string(w) + "x" + std::to_string(h) + ")";
        m_decodeInfoLabel->copy_label(info.c_str());
        m_decodeStatusBox->copy_label("Изображение загружено. Нажмите 'Извлечь скрытые данные'.");
        m_decodeStatusBox->labelcolor(COLOR_MUTED);

        m_decodeTextBuf->text("");
        m_decodeCopyBtn->deactivate();
        m_decodeSaveFileBtn->deactivate();
    }
}

void MainWindow::onDecode() {
    if (!m_decodeImgBox->hasImage()) {
        fl_alert("Пожалуйста, сначала выберите Stego-изображение.");
        return;
    }

    std::string password = m_decodePasswordInput->value() ? m_decodePasswordInput->value() : "";
    std::string err;

    int w = m_decodeImgBox->getImageWidth();
    int h = m_decodeImgBox->getImageHeight();
    int ch = m_decodeImgBox->getImageChannels();

    m_extractedPayload = StegoPayload{};
    bool ok = StegoEngine::extract(m_decodeImgBox->getRawPixels(), w, h, ch, password, m_extractedPayload, err);

    if (!ok) {
        m_decodeResultCard->copy_label("  Контейнер: Ошибка извлечения");
        m_decodeStatusBox->copy_label(("  " + err).c_str());
        m_decodeStatusBox->labelcolor(COLOR_ERROR);
        fl_alert("Не удалось извлечь данные:\n%s", err.c_str());
        return;
    }

    m_decodeStatusBox->copy_label("  Данные успешно извлечены и проверены (CRC-32 OK)!");
    m_decodeStatusBox->labelcolor(COLOR_SUCCESS);

    if (m_extractedPayload.isFile) {
        std::string cardText = "  Контейнер: PixelVault v1\n  Вложенный файл: " +
                               m_extractedPayload.filename + " (" +
                               formatBytes(m_extractedPayload.data.size()) + ")";
        m_decodeResultCard->copy_label(cardText.c_str());
        m_decodeTextBuf->text("[Внедрен двоичный файл. Нажмите кнопку ниже для сохранения на диск]");
        m_decodeCopyBtn->deactivate();
        m_decodeSaveFileBtn->activate();
    } else {
        std::string textContent(m_extractedPayload.data.begin(), m_extractedPayload.data.end());
        std::string cardText = "  Контейнер: PixelVault v1\n  Текстовое сообщение: " +
                               formatBytes(m_extractedPayload.data.size()) + " (" +
                               std::to_string(textContent.size()) + " симв.)";
        m_decodeResultCard->copy_label(cardText.c_str());
        m_decodeTextBuf->text(textContent.c_str());
        m_decodeCopyBtn->activate();
        m_decodeSaveFileBtn->deactivate();
    }
}

void MainWindow::onCopyDecodedText() {
    char *text = m_decodeTextBuf->text();
    if (text) {
        Fl::copy(text, std::strlen(text), 1);
        Fl::copy(text, std::strlen(text), 0);
        m_decodeStatusBox->copy_label("  Текст скопирован в буфер обмена!");
        m_decodeStatusBox->labelcolor(COLOR_SUCCESS);
        free(text);
    }
}

void MainWindow::onSaveDecodedFile() {
    if (m_extractedPayload.data.empty() || !m_extractedPayload.isFile) return;

    Fl_Native_File_Chooser fnfc;
    fnfc.title("Сохранить извлеченный файл");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fnfc.preset_file(m_extractedPayload.filename.c_str());

    if (fnfc.show() == 0) {
        std::string outPath = fnfc.filename();
        std::ofstream out(outPath, std::ios::binary);
        if (out.is_open()) {
            out.write(reinterpret_cast<const char*>(m_extractedPayload.data.data()),
                      m_extractedPayload.data.size());
            out.close();
            fl_message("Файл успешно сохранен:\n%s", outPath.c_str());
            m_decodeStatusBox->copy_label(("  Файл сохранен: " + getFilenameOnly(outPath)).c_str());
            m_decodeStatusBox->labelcolor(COLOR_SUCCESS);
        } else {
            fl_alert("Не удалось сохранить файл на диск.");
        }
    }
}

void MainWindow::onSelectAnalysisImage() {
    Fl_Native_File_Chooser fnfc;
    fnfc.title("Выберите изображение для анализа");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fnfc.filter("Изображения\t*.{png,jpg,jpeg,bmp}");

    if (fnfc.show() == 0) {
        std::string path = fnfc.filename();
        Fl_Shared_Image *img = Fl_Shared_Image::get(path.c_str());
        if (!img || img->w() == 0) {
            fl_alert("Не удалось загрузить выбранное изображение.");
            return;
        }

        m_analysisW = img->w();
        m_analysisH = img->h();
        m_analysisCh = img->d();
        size_t sz = static_cast<size_t>(m_analysisW) * m_analysisH * m_analysisCh;
        m_analysisRawOrig.resize(sz);
        std::memcpy(m_analysisRawOrig.data(), img->data()[0], sz);

        m_analysisImgBox->setImage(m_analysisRawOrig.data(), m_analysisW, m_analysisH, m_analysisCh);
        m_analysisInfoLabel->copy_label((getFilenameOnly(path) + " (" + std::to_string(m_analysisW) + "x" + std::to_string(m_analysisH) + ")").c_str());
    }
}

void MainWindow::onAnalysisModeChanged(int mode) {
    if (m_analysisRawOrig.empty()) return;

    if (mode == 0) {
        // Original view
        m_analysisImgBox->setImage(m_analysisRawOrig.data(), m_analysisW, m_analysisH, m_analysisCh);
    } else {
        int bitIdx = (mode == 1) ? 0 : 1;
        std::vector<uint8_t> planePixels(m_analysisRawOrig.size());
        StegoEngine::extractBitPlane(m_analysisRawOrig.data(), planePixels.data(),
                                    m_analysisW, m_analysisH, m_analysisCh, bitIdx);
        m_analysisImgBox->setImage(planePixels.data(), m_analysisW, m_analysisH, m_analysisCh);
    }
}

#include "MainWindow.h"
#include "qrcodegen.hpp"
#include "basen.hpp"
#include <regex>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QDialog>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QLinearGradient>

static bool isValidBase64(const std::string &s) {
    static const std::regex b64Pattern(R"(^[A-Za-z0-9+/]+={0,2}$)");
    return (s.size() % 4 == 0) && std::regex_match(s, b64Pattern);
}

static std::string encodeBase64(const std::string &input) {
    std::string output;
    bn::encode_b64(input.begin(), input.end(), std::back_inserter(output));
    return output;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      primaryColor(Qt::black),
      secondaryColor(Qt::blue)
{
    // стартовый список закодированных Base64 ссылок
    memeLinks = {
        "aHR0cHM6Ly92a3ZpZGVvLnJ1L3ZpZGVvLTE3NzMwNDU0OV80NTYyMzk4Nzg=",
        "aHR0cHM6Ly95b3V0dS5iZS90bWFtZW0="
    };

    auto *central = new QWidget;
    auto *mainLayout = new QVBoxLayout;

    inputEdit = new QLineEdit;
    inputEdit->setPlaceholderText("Введите текст для QR‑кода");
    mainLayout->addWidget(inputEdit);

    memeCheckBox = new QCheckBox("Включить мемы");
    memeCheckBox->setChecked(true);
    mainLayout->addWidget(memeCheckBox);

    // Кнопка вызова диалога редактирования ссылок
    editLinksButton = new QPushButton("Редактировать ссылки мемов");
    connect(editLinksButton, &QPushButton::clicked,
            this, &MainWindow::editMemeLinks);
    mainLayout->addWidget(editLinksButton);

    // Ползунок вероятности
    auto *probLayout = new QHBoxLayout;
    probabilityLabel = new QLabel("Вероятность мемов: 40%");
    probabilitySlider = new QSlider(Qt::Horizontal);
    probabilitySlider->setRange(0, 100);
    probabilitySlider->setValue(40);
    connect(probabilitySlider, &QSlider::valueChanged,
            this, &MainWindow::updateProbabilityLabel);
    probLayout->addWidget(probabilityLabel);
    probLayout->addWidget(probabilitySlider);
    mainLayout->addLayout(probLayout);

    auto *styleLayout = new QHBoxLayout;
    styleLayout->addWidget(new QLabel("Стиль QR:"));
    styleCombo = new QComboBox;
    styleCombo->addItems({ "Square", "Circle", "Rounded" });
    styleLayout->addWidget(styleCombo);
    mainLayout->addLayout(styleLayout);

    // Элементы для выбора цвета QR‑кода
    auto *colorLayout = new QHBoxLayout;
    primaryColorButton = new QPushButton("Основной цвет");
    primaryColorButton->setStyleSheet(QString("background-color: %1").arg(primaryColor.name()));
    connect(primaryColorButton, &QPushButton::clicked, this, &MainWindow::selectPrimaryColor);
    colorLayout->addWidget(primaryColorButton);

    gradientCheck = new QCheckBox("Градиент");
    gradientCheck->setChecked(false);
    colorLayout->addWidget(gradientCheck);

    secondaryColorButton = new QPushButton("Вторичный цвет");
    secondaryColorButton->setStyleSheet(QString("background-color: %1").arg(secondaryColor.name()));
    secondaryColorButton->setEnabled(false);
    connect(secondaryColorButton, &QPushButton::clicked, this, &MainWindow::selectSecondaryColor);
    colorLayout->addWidget(secondaryColorButton);

    connect(gradientCheck, &QCheckBox::toggled, secondaryColorButton, &QPushButton::setEnabled);

    mainLayout->addLayout(colorLayout);

    generateButton = new QPushButton("Сгенерировать QR‑код");
    connect(generateButton, &QPushButton::clicked,
            this, &MainWindow::generateQrCode);
    mainLayout->addWidget(generateButton);

    // Кнопка сохранения QR‑кода
    saveButton = new QPushButton("Сохранить QR‑код");
    connect(saveButton, &QPushButton::clicked,
            this, &MainWindow::saveQrCode);
    mainLayout->addWidget(saveButton);

    qrImageLabel = new QLabel;
    qrImageLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(qrImageLabel);

    central->setLayout(mainLayout);
    setCentralWidget(central);
}

MainWindow::~MainWindow() { }

void MainWindow::updateProbabilityLabel(int value) {
    probabilityLabel->setText(
        QString("Вероятность мемов: %1%").arg(value)
    );
}

void MainWindow::generateQrCode() {
    QString userText = inputEdit->text().trimmed();
    std::string textToEncode;

    if (memeCheckBox->isChecked() && !userText.isEmpty()) {
        int prob = probabilitySlider->value();
        int r = arc4random_uniform(100);
        if (r < prob && !memeLinks.empty()) {
            // выбираем случайную ссылку из списка
            int idx = arc4random_uniform(static_cast<quint32>(memeLinks.size()));
            std::string decoded;
            bn::decode_b64(memeLinks[idx].begin(),
                           memeLinks[idx].end(),
                           std::back_inserter(decoded));
            textToEncode = decoded;
        } else {
            textToEncode = userText.toStdString();
        }
    } else {
        textToEncode = userText.toStdString();
    }

    if (textToEncode.empty())
        return;

    auto qr = qrcodegen::QrCode::encodeText(
        textToEncode.c_str(),
        qrcodegen::QrCode::Ecc::MEDIUM
    );

    const int margin = 4;
    const int scale  = 10;
    const int size   = qr.getSize();
    const int imgSize = (size + margin*2) * scale;

    QImage image(imgSize, imgSize, QImage::Format_ARGB32);
    image.fill(Qt::white);

    if (!gradientCheck->isChecked()) {
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(primaryColor);
        painter.setPen(Qt::NoPen);

        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                if (!qr.getModule(x, y)) continue;
                int px = (x + margin) * scale;
                int py = (y + margin) * scale;
                QString style = styleCombo->currentText();
                if (style == "Square") {
                    painter.drawRect(px, py, scale, scale);
                } else if (style == "Circle") {
                    painter.drawEllipse(px, py, scale, scale);
                } else if (style == "Rounded") {
                    int r = scale / 4;
                    painter.drawRoundedRect(px, py, scale, scale, r, r);
                }
            }
        }
        painter.end();
    } else {
        // Режим градиента: создаём маску и градиентное изображение, затем композитим
        QImage maskImage(imgSize, imgSize, QImage::Format_ARGB32);
        maskImage.fill(Qt::transparent);
        {
            QPainter maskPainter(&maskImage);
            maskPainter.setRenderHint(QPainter::Antialiasing, true);
            maskPainter.setBrush(Qt::white);
            maskPainter.setPen(Qt::NoPen);
            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    if (!qr.getModule(x, y)) continue;
                    int px = (x + margin) * scale;
                    int py = (y + margin) * scale;
                    QString style = styleCombo->currentText();
                    if (style == "Square") {
                        maskPainter.drawRect(px, py, scale, scale);
                    } else if (style == "Circle") {
                        maskPainter.drawEllipse(px, py, scale, scale);
                    } else if (style == "Rounded") {
                        int r = scale / 4;
                        maskPainter.drawRoundedRect(px, py, scale, scale, r, r);
                    }
                }
            }
            maskPainter.end();
        }

        QImage gradientImage(imgSize, imgSize, QImage::Format_ARGB32);
        {
            QPainter gradPainter(&gradientImage);
            QLinearGradient gradient(0, 0, imgSize, imgSize);
            gradient.setColorAt(0, primaryColor);
            gradient.setColorAt(1, secondaryColor);
            gradPainter.fillRect(0, 0, imgSize, imgSize, gradient);
            gradPainter.end();
        }

        QImage resultImage(imgSize, imgSize, QImage::Format_ARGB32);
        resultImage.fill(Qt::transparent);
        {
            QPainter resPainter(&resultImage);
            resPainter.drawImage(0,0, gradientImage);
            resPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            resPainter.drawImage(0,0, maskImage);
            resPainter.end();
        }

        QPainter finalPainter(&image);
        finalPainter.drawImage(0,0, resultImage);
        finalPainter.end();
    }

    qrImageLabel->setPixmap(
        QPixmap::fromImage(image)
            .scaled(qrImageLabel->size(),
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation)
    );
    currentQrImage = image;
}

void MainWindow::editMemeLinks() {
    QDialog dlg(this);
    dlg.setWindowTitle("Редактирование ссылок мемов");
    auto *layout = new QVBoxLayout(&dlg);

    // Список текущих Base64-строк
    auto *listWidget = new QListWidget;
    for (const auto &link : memeLinks) {
        auto *item = new QListWidgetItem(QString::fromStdString(link));
        std::string decoded;
        bn::decode_b64(link.begin(), link.end(), std::back_inserter(decoded));
        item->setToolTip(QString::fromStdString(decoded));
        listWidget->addItem(item);
    }
    layout->addWidget(listWidget);

    auto *hl = new QHBoxLayout;
    auto *lineEdit = new QLineEdit;
    lineEdit->setPlaceholderText("Новая ссылка или Base64");
    hl->addWidget(lineEdit);

    auto *btnAdd = new QPushButton("Добавить");
    hl->addWidget(btnAdd);
    connect(btnAdd, &QPushButton::clicked, [&]() {
        QString text = lineEdit->text().trimmed();
        if (text.isEmpty())
            return;

        std::string raw = text.toStdString();
        std::string stored = isValidBase64(raw) ? raw : encodeBase64(raw);

        auto *item = new QListWidgetItem(QString::fromStdString(stored));
        std::string decoded;
        bn::decode_b64(stored.begin(), stored.end(), std::back_inserter(decoded));
        item->setToolTip(QString::fromStdString(decoded));

        listWidget->addItem(item);
        lineEdit->clear();
    });

    auto *btnRemove = new QPushButton("Удалить выбранную");
    hl->addWidget(btnRemove);
    connect(btnRemove, &QPushButton::clicked, [&]() {
        for (auto *item : listWidget->selectedItems())
            delete listWidget->takeItem(listWidget->row(item));
    });
    layout->addLayout(hl);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(bb);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        memeLinks.clear();
        for (int i = 0; i < listWidget->count(); ++i)
            memeLinks.push_back(listWidget->item(i)->text().toStdString());
    }
}

void MainWindow::selectPrimaryColor() {
    QColor color = QColorDialog::getColor(primaryColor, this, tr("Выберите основной цвет"));
    if (color.isValid()) {
        primaryColor = color;
        primaryColorButton->setStyleSheet(QString("background-color: %1").arg(color.name()));
    }
}

void MainWindow::selectSecondaryColor() {
    QColor color = QColorDialog::getColor(secondaryColor, this, tr("Выберите вторичный цвет"));
    if (color.isValid()) {
        secondaryColor = color;
        secondaryColorButton->setStyleSheet(QString("background-color: %1").arg(color.name()));
    }
}

void MainWindow::saveQrCode() {
    if (currentQrImage.isNull())
        return;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить QR‑код"), "", tr("PNG Images (*.png)"));
    if (!fileName.isEmpty()) {
        currentQrImage.save(fileName);
    }
}

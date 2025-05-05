#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <string>
#include <QImage>
#include <QColor>

class QLineEdit;
class QLabel;
class QPushButton;
class QCheckBox;
class QSlider;
class QComboBox;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateProbabilityLabel(int value);
    void generateQrCode();
    void editMemeLinks();

    // Новые слоты
    void selectPrimaryColor();
    void selectSecondaryColor();
    void saveQrCode();

private:
    QLineEdit          *inputEdit;
    QCheckBox          *memeCheckBox;
    QLabel             *probabilityLabel;
    QSlider            *probabilitySlider;
    QPushButton        *generateButton;
    QPushButton        *editLinksButton;
    QLabel             *qrImageLabel;
    QComboBox          *styleCombo;      // выбор стиля

    // Новые элементы UI для выбора цвета и сохранения
    QPushButton        *saveButton;
    QPushButton        *primaryColorButton;
    QPushButton        *secondaryColorButton;
    QCheckBox          *gradientCheck;

    // Цвета для QR‑кода
    QColor              primaryColor;
    QColor              secondaryColor;

    // Сгенерированное изображение QR‑кода для сохранения
    QImage              currentQrImage;

    std::vector<std::string> memeLinks;
};

#endif // MAINWINDOW_H

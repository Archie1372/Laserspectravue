#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts>
#include <QTimer>
#include <QPushButton>
#include <QLineEdit>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPointF>
#include <QGraphicsPixmapItem>
#include <QList>
#include "ftd2xx.h"
#include <memory>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QSpinBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSvgGenerator>
#include <QPainter>

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void updatePlot();

    void startDataAcquisition();
    void stopDataAcquisition();
    void onSetExposureClicked();
    void onSaveDataClicked();
    void onSetBackgroundClicked();
    void onToggleSubtractedValuesView();
    void onSetRangeClicked();
    void onToggleAverageView();  // Added this line
    void onStoreTraceClicked();

private:

    QPushButton *toggleYRangeButton = nullptr;
    QSpinBox *minYRangeSpinBox = nullptr;
    QSpinBox *maxYRangeSpinBox = nullptr;
    bool isAutoYRange = true;
    double userMinYRange = 0;
    double userMaxYRange = 65535;  // Assuming 16-bit sensor

    // Add this new private slot
    void onToggleYRangeClicked();
    static int findFrameStart(const QByteArray& data);
    void saveChartImage();
    void updateAllSeriesWithNewRange();
    void saveAsCSVorTXT(QTextStream& out, bool saveAllFrames, const QString& extension);
    void saveAsJSON(QTextStream& out, bool saveAllFrames);
    QByteArray dataBuffer;
    static constexpr int MAX_STORED_TRACES = 5;  // Maximum number of stored traces
    QVector<QVector<QPointF>> storedTraces;  // Container for stored traces
    QVector<QSharedPointer<QLineSeries>> storedSeries; // Series for stored traces
    QPushButton *storeTraceButton = nullptr;  // Button to store current trace
    QLabel *saturationIndicator = nullptr;
    void updateSaturationIndicator(bool isSaturating) const;
    QPushButton *showAverageButton = nullptr;
    bool showingAverage = false;
    QVector<QVector<QPointF>> lastTenFrames;
    void updateAveragePlot();
    QVector<QPointF> processFrame(const QByteArray &frameData);
    void updatePlotWithPoints(const QVector<QPointF>& points) const;  // Added this line
    QVector<QPointF> filterPointsByRange(const QVector<QPointF> &points) const;

    void updateMainSeries(const QVector<QPointF> &filteredPoints) const;

    void updatePeakIndicator(const QVector<QPointF> &filteredPoints) const;

    void updateAxisRanges(const QVector<QPointF> &filteredPoints) const;

    void updateLabels(const QVector<QPointF> &filteredPoints) const;

    QVector<QVector<QPointF>> allFramesData;
    bool isRecording = false;
    void startRecording();
    void stopRecording();
    void saveAllFrames();

    static QString buttonStyle(const QString& color = "");
    static void setupButton(QPushButton* button, const QString& iconPath, const QString& tooltip);
    QLabel* createStylishLabel(const QString& text);
    void connectSignalsAndSlots();
    void setupTimer();
    FT_HANDLE ftHandle = nullptr;
    FT_HANDLE fthandle_uart = nullptr;
    FT_STATUS ft_status{};
    QByteArray frameData;
    QLabel *peakValueLabel{};
    QLabel *peakPixelLabel{};
    QLabel *peakToPeakValueLabel{};
    std::unique_ptr<QChart> chart;
    std::unique_ptr<QLineSeries> series;
    std::unique_ptr<QLineSeries> peakLineSeries;
    QLabel *amplitudeLabel{};
    QLabel *meanLabel{};
    QLabel *stdDevLabel{};
    QLabel *fwhmLabel{};
    // Add these new labels
    QLabel *varianceLabel{};
    QLabel *medianLabel{};

    // Add the Statistics struct
    struct Statistics {
        double variance{0.0};
        double stdDev{0.0};
        double mean{0.0};
        double median{0.0};
    };

    // Add the calculation function
    Statistics calculateStatistics(const QVector<QPointF>& points) const;

    static constexpr int Buffer_Size = 2088;
    static constexpr int Display_Frame_Count = 1;
    static constexpr int expectedFrameSize = 2088;
    static constexpr int maxBufferSize = expectedFrameSize * 10;
    QQueue<QVector<QPointF>> frameBuffer;
    DWORD readbyte{};
    DWORD writebyte{};
    int currentFrame = 0;

    QChartView *chartView = nullptr;
    QTimer *timer = nullptr;

    QPushButton *startButton = nullptr;
    QPushButton *stopButton = nullptr;
    QLineEdit *exposureTimeInput = nullptr;
    QPushButton *setExposureButton = nullptr;
    QPushButton *saveDataButton = nullptr;
    QPushButton *setBackgroundButton = nullptr;
    QPushButton *showSubtractedValuesButton = nullptr;

    QSpinBox *minRangeSpinBox = nullptr;
    QSpinBox *maxRangeSpinBox = nullptr;
    QPushButton *setRangeButton = nullptr;
    int currentMinRange = 0;
    int currentMaxRange = 1023;

    uint32_t defaultExposureTime = 10000;
    uint8_t buffer[2088]{};
    QVector<QPointF> backgroundData;
    bool showSubtracted = false;

    void setupDevice();
    void setupDeviceHelper(const char* description, FT_HANDLE* handle);

    void setupChart();
    void setupUI();
    void trig_on() const;
    void trig_off() const;
    void set_exp(uint32_t exp) const;
    static void logError(const QString &message);
    static void showDiagnosticMessage(const QString& message, const QString& type);

    QStatusBar* statusBar{nullptr};
    void updateStatusBar(const QString& message, int timeout = 3000);
    void setupShortcuts();

    // Keyboard shortcuts
    QShortcut* startShortcut{nullptr};
    QShortcut* stopShortcut{nullptr};
    QShortcut* saveShortcut{nullptr};
    QShortcut* toggleAverageShortcut{nullptr};
    QShortcut* setBackgroundShortcut{nullptr};
    QShortcut* storeTraceShortcut{nullptr};



};

#endif // MAINWINDOW_H
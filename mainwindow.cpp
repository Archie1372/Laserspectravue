#include "mainwindow.h"
#include <QDebug>
#include <memory> // Include for std::unique_ptr






MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ftHandle(nullptr),
    fthandle_uart(nullptr),
    chart(std::make_unique<QChart>()),
    series(std::make_unique<QLineSeries>()),
    peakLineSeries(std::make_unique<QLineSeries>()),
    defaultExposureTime(10000)
{
    setWindowTitle("MDSpectra");

    // Create and set up status bar first
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    // Set up UI elements first so window appears responsive
    setupChart();
    setupUI();
    setupShortcuts();

    // Show the window immediately
    show();

    // Use QTimer to delay device initialization
    QTimer::singleShot(100, this, [this]() {
        try {
            setupDevice();
            updateStatusBar(tr("Application initialized successfully"));
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Device Error",
                QString("Failed to initialize devices: %1").arg(e.what()));
            updateStatusBar(tr("Failed to initialize devices"), 5000);
        }
    });
}

MainWindow::~MainWindow() {
    if (ftHandle != nullptr) FT_Close(ftHandle);
    if (fthandle_uart != nullptr) FT_Close(fthandle_uart);

}

void MainWindow::setupDevice() {
    try {
        setupDeviceHelper("MD_HS_V1 A", &fthandle_uart);
        setupDeviceHelper("MD_HS_V1 B", &ftHandle);

        if (ft_status != FT_OK) {
            throw std::runtime_error("Failed to open device");
        }

        FT_STATUS status = FT_SetBaudRate(fthandle_uart, 9600);
        if (status != FT_OK) {
            throw std::runtime_error("Failed to set baud rate");
        }

        status = FT_ResetDevice(fthandle_uart);
        if (status != FT_OK) {
            throw std::runtime_error("Failed to reset device");
        }

        trig_off(); // To turn off
        set_exp(defaultExposureTime);
        qDebug() << "**Device setup complete**";
    }
    catch (const std::exception& e) {
        qDebug() << "Device setup failed:" << e.what();
        throw; // Re-throw to be caught by the caller
    }
}

void MainWindow::setupDeviceHelper(const char* description, FT_HANDLE* handle) {
    if (*handle != nullptr) {
        FT_Close(*handle);
        *handle = nullptr;
    }

    // Correctly cast using const_cast and reinterpret_cast
    ft_status = FT_OpenEx(reinterpret_cast<PVOID>(const_cast<char*>(description)), FT_OPEN_BY_DESCRIPTION, handle);

    if (ft_status != FT_OK) {
        logError("Failed to open " + QString(description) + " device. Error: " + QString::number(ft_status));
    }
}




void MainWindow::setupChart() {
    series->setUseOpenGL(true);
    chart->addSeries(series.get());
    chart->setTitle("Live Data Plot");
    chart->setTitleFont(QFont("Arial", 12, QFont::Bold));
    chart->setTitleBrush(QColor(255, 255, 255));  // Black title

    auto axisX = new QValueAxis(chart.get());
    axisX->setLabelFormat("%i");
    axisX->setTitleText("Pixel");
    axisX->setTitleFont(QFont("Arial", 10));
    axisX->setLabelsFont(QFont("Arial", 8));
    axisX->setTitleBrush(QColor(255, 255, 255));  // Black title
    axisX->setLabelsBrush(QColor(255, 255, 255)); // Black labels
    axisX->setLinePenColor(QColor(255, 255, 255)); // Black axis line
    axisX->setGridLineColor(QColor(200, 200, 200)); // Light gray grid lines
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    auto axisY = new QValueAxis(chart.get());
    axisY->setLabelFormat("%i");
    axisY->setTitleText("Intensity");
    axisY->setTitleFont(QFont("Arial", 10));
    axisY->setLabelsFont(QFont("Arial", 8));
    axisY->setTitleBrush(QColor(255, 255, 255));  // Black title
    axisY->setLabelsBrush(QColor(255, 255, 255)); // Black labels
    axisY->setLinePenColor(QColor(255, 255, 255)); // Black axis line
    axisY->setGridLineColor(QColor(200, 200, 200)); // Light gray grid lines
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Set a dark color for the line series for better visibility on white background
    QPen seriesPen(QColor(0, 0, 255));  // Blue color
    seriesPen.setWidth(2);  // Increase line thickness
    series->setPen(seriesPen);

    // Set up the peak line series
    QPen peakPen(Qt::red);
    peakPen.setWidth(2);
    peakLineSeries->setPen(peakPen);
    chart->addSeries(peakLineSeries.get());
    peakLineSeries->attachAxis(axisX);
    peakLineSeries->attachAxis(axisY);

    // Set chart background to white
    chart->setBackgroundBrush(QColor(255, 255, 255));
    chart->setPlotAreaBackgroundBrush(QColor(255, 255, 255));
    chart->setPlotAreaBackgroundVisible(true);

    // Add some padding to the plot area
    chart->setMargins(QMargins(1, 1, 1, 1));

    chartView = new QChartView(chart.get(), this);
    chartView->setRenderHint(QPainter::Antialiasing);
}


void MainWindow::setupUI() {
    // Set the application style to Fusion for a clean base
    qApp->setStyle(QStyleFactory::create("Fusion"));

    // Set a refined dark color palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(18, 18, 18));
    darkPalette.setColor(QPalette::WindowText, QColor(236, 236, 236));
    darkPalette.setColor(QPalette::Base, QColor(24, 24, 24));
    darkPalette.setColor(QPalette::AlternateBase, QColor(18, 18, 18));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(236, 236, 236));
    darkPalette.setColor(QPalette::ToolTipText, QColor(236, 236, 236));
    darkPalette.setColor(QPalette::Text, QColor(236, 236, 236));
    darkPalette.setColor(QPalette::Button, QColor(36, 36, 36));
    darkPalette.setColor(QPalette::ButtonText, QColor(236, 236, 236));
    darkPalette.setColor(QPalette::BrightText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
    qApp->setPalette(darkPalette);

   auto mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);

    auto mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setSpacing(2); // Reduced spacing
    mainLayout->setContentsMargins(2, 2, 2, 2); // Reduced margins

    auto headerLabel = new QLabel("MDspecView", this);
    headerLabel->setAlignment(Qt::AlignCenter);
    headerLabel->setStyleSheet(R"(
        font-family: 'Segoe UI', Arial, sans-serif;
        font-size: 12px;
        font-weight: 700;
        color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4CAF50, stop:1 #2196F3);
        margin-bottom: 2px;
    )");
    mainLayout->addWidget(headerLabel);

    auto chartContainer = new QWidget(this);
    chartContainer->setObjectName("chartContainer");
    chartContainer->setStyleSheet(R"(
        #chartContainer {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                        stop:0 #2C2C2C, stop:1 #1A1A1A);
            border-radius: 2px;
            padding: 2px;
        }
    )");
    auto chartLayout = new QVBoxLayout(chartContainer);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(960, 480);
    chartView->chart()->setBackgroundBrush(QColor(24, 24, 24));
    chartView->chart()->setTitleBrush(QColor(236, 236, 236));
    chartView->chart()->legend()->setLabelColor(QColor(236, 236, 236));
    chartLayout->addWidget(chartView);
    mainLayout->addWidget(chartContainer);

    auto controlsContainer = new QWidget(this);
    controlsContainer->setObjectName("controlsContainer");
    controlsContainer->setStyleSheet(R"(
        #controlsContainer {
            background-color: rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 12px;
            padding: 20px;
        }
    )");
    auto controlsLayout = new QHBoxLayout(controlsContainer);
    controlsLayout->setSpacing(20);

    auto exposureLayout = new QVBoxLayout();
    auto exposureLabel = new QLabel("Exposure Time (μs)", this);
    exposureLabel->setStyleSheet("color: #BBBBBB; font-weight: 500; font-size: 14px;");
    exposureTimeInput = new QLineEdit(this);
    exposureTimeInput->setText(QString::number(defaultExposureTime));
    exposureTimeInput->setStyleSheet(R"(
        background-color: rgba(255, 255, 255, 0.1);
        color: #FFFFFF;
        border: none;
        border-radius: 6px;
        padding: 8px;
        font-size: 14px;
    )");
    setExposureButton = new QPushButton("Set", this);
    setExposureButton->setStyleSheet(buttonStyle());
    exposureLayout->addWidget(exposureLabel);
    exposureLayout->addWidget(exposureTimeInput);
    exposureLayout->addWidget(setExposureButton);
    controlsLayout->addLayout(exposureLayout);

    auto buttonsLayout = new QHBoxLayout();
    startButton = new QPushButton("Start", this);
    stopButton = new QPushButton("Stop", this);
    startButton->setStyleSheet(buttonStyle("green"));
    stopButton->setStyleSheet(buttonStyle("red"));
    stopButton->setEnabled(false);
    buttonsLayout->addWidget(startButton);
    buttonsLayout->addWidget(stopButton);
    controlsLayout->addLayout(buttonsLayout);

    auto additionalButtonsLayout = new QHBoxLayout();
    saveDataButton = new QPushButton(this);
    setBackgroundButton = new QPushButton(this);
    showSubtractedValuesButton = new QPushButton(this);
    showAverageButton = new QPushButton(this);
    storeTraceButton = new QPushButton(this);

    setupButton(saveDataButton, ":/resources/save.png", "Save Data");
    setupButton(setBackgroundButton, ":/resources/off.png", "Set As Background");
    setupButton(showSubtractedValuesButton, ":/resources/on.png", "Toggle Subtracted Values");
    setupButton(showAverageButton, ":/resources/average.png", "Toggle Average View");
    setupButton(storeTraceButton, ":/resources/hold.png", "Store Current Trace");

    additionalButtonsLayout->addWidget(saveDataButton);
    additionalButtonsLayout->addWidget(setBackgroundButton);
    additionalButtonsLayout->addWidget(showSubtractedValuesButton);
    additionalButtonsLayout->addWidget(showAverageButton);
    additionalButtonsLayout->addWidget(storeTraceButton);
    controlsLayout->addLayout(additionalButtonsLayout);

    mainLayout->addWidget(controlsContainer);

    auto rangeContainer = new QWidget(this);
    rangeContainer->setObjectName("rangeContainer");
    rangeContainer->setStyleSheet(R"(
        #rangeContainer {
            background-color: rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 12px;
            padding: 20px;
        }
    )");
    auto rangeLayout = new QHBoxLayout(rangeContainer);
    rangeLayout->setSpacing(20);

    auto minRangeLabel = new QLabel("Min Range:", this);
    minRangeLabel->setStyleSheet("color: #BBBBBB; font-weight: 500; font-size: 14px;");
    minRangeSpinBox = new QSpinBox(this);
    minRangeSpinBox->setRange(0, 1023);
    minRangeSpinBox->setValue(0);
    minRangeSpinBox->setStyleSheet(R"(
        background-color: rgba(255, 255, 255, 0.1);
        color: #FFFFFF;
        border: none;
        border-radius: 6px;
        padding: 8px;
        font-size: 14px;
    )");

    auto maxRangeLabel = new QLabel("Max Range:", this);
    maxRangeLabel->setStyleSheet("color: #BBBBBB; font-weight: 500; font-size: 14px;");
    maxRangeSpinBox = new QSpinBox(this);
    maxRangeSpinBox->setRange(0, 1023);
    maxRangeSpinBox->setValue(1023);
    maxRangeSpinBox->setStyleSheet(R"(
        background-color: rgba(255, 255, 255, 0.1);
        color: #FFFFFF;
        border: none;
        border-radius: 6px;
        padding: 8px;
        font-size: 14px;
    )");

    setRangeButton = new QPushButton("Set Range", this);
    setRangeButton->setStyleSheet(buttonStyle());

    rangeLayout->addWidget(minRangeLabel);
    rangeLayout->addWidget(minRangeSpinBox);
    rangeLayout->addWidget(maxRangeLabel);
    rangeLayout->addWidget(maxRangeSpinBox);
    rangeLayout->addWidget(setRangeButton);

    mainLayout->addWidget(rangeContainer);

    auto yRangeContainer = new QWidget(this);
    yRangeContainer->setObjectName("yRangeContainer");
    yRangeContainer->setStyleSheet(R"(
        #yRangeContainer {
            background-color: rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 12px;
            padding: 20px;
        }
    )");
    auto yRangeLayout = new QHBoxLayout(yRangeContainer);
    yRangeLayout->setSpacing(20);

    toggleYRangeButton = new QPushButton("Auto Y Range", this);
    toggleYRangeButton->setCheckable(true);
    toggleYRangeButton->setChecked(true);
    toggleYRangeButton->setStyleSheet(buttonStyle());

    auto minYRangeLabel = new QLabel("Min Y:", this);
    minYRangeLabel->setStyleSheet("color: #BBBBBB; font-weight: 500; font-size: 14px;");
    minYRangeSpinBox = new QSpinBox(this);
    minYRangeSpinBox->setRange(0, 65535);
    minYRangeSpinBox->setValue(0);
    minYRangeSpinBox->setEnabled(false);
    minYRangeSpinBox->setStyleSheet(R"(
        background-color: rgba(255, 255, 255, 0.1);
        color: #FFFFFF;
        border: none;
        border-radius: 6px;
        padding: 8px;
        font-size: 14px;
    )");

    auto maxYRangeLabel = new QLabel("Max Y:", this);
    maxYRangeLabel->setStyleSheet("color: #BBBBBB; font-weight: 500; font-size: 14px;");
    maxYRangeSpinBox = new QSpinBox(this);
    maxYRangeSpinBox->setRange(0, 65535);
    maxYRangeSpinBox->setValue(65535);
    maxYRangeSpinBox->setEnabled(false);
    maxYRangeSpinBox->setStyleSheet(R"(
        background-color: rgba(255, 255, 255, 0.1);
        color: #FFFFFF;
        border: none;
        border-radius: 6px;
        padding: 8px;
        font-size: 14px;
    )");

    yRangeLayout->addWidget(toggleYRangeButton);
    yRangeLayout->addWidget(minYRangeLabel);
    yRangeLayout->addWidget(minYRangeSpinBox);
    yRangeLayout->addWidget(maxYRangeLabel);
    yRangeLayout->addWidget(maxYRangeSpinBox);

    mainLayout->addWidget(yRangeContainer);

    auto labelsContainer = new QWidget(this);
    labelsContainer->setObjectName("labelsContainer");
    labelsContainer->setStyleSheet(R"(
    #labelsContainer {
        background-color: rgba(255, 255, 255, 0.03);
        border-radius: 12px;
        padding: 16px;
    }
)");

    // Create two horizontal layouts for better organization
    auto labelsMainLayout = new QVBoxLayout(labelsContainer);
    auto labelsTopLayout = new QHBoxLayout();
    auto labelsBottomLayout = new QHBoxLayout();
    labelsMainLayout->addLayout(labelsTopLayout);
    labelsMainLayout->addLayout(labelsBottomLayout);

    // Basic measurements in top layout
    peakValueLabel = createStylishLabel("Peak Value: N/A");
    peakPixelLabel = createStylishLabel("Peak Pixel: N/A");
    peakToPeakValueLabel = createStylishLabel("Peak to Peak: N/A");
    saturationIndicator = new QLabel(this);
    saturationIndicator->setFixedSize(20, 20);
    saturationIndicator->setStyleSheet("background-color: green; border-radius: 20px;");
    saturationIndicator->setToolTip("Camera is operating normally");

    // Statistical measurements in bottom layout
    varianceLabel = createStylishLabel("Variance: N/A");
    stdDevLabel = createStylishLabel("Std Dev: N/A");
    meanLabel = createStylishLabel("Mean: N/A");
    medianLabel = createStylishLabel("Median: N/A");

    // Add tooltips for the statistical labels
    varianceLabel->setToolTip("Measure of variability in the signal");
    stdDevLabel->setToolTip("Standard deviation of the signal intensity");
    meanLabel->setToolTip("Average signal intensity");
    medianLabel->setToolTip("Middle value of sorted intensities");

    // Add widgets to layouts
    labelsTopLayout->addWidget(peakValueLabel);
    labelsTopLayout->addWidget(peakPixelLabel);
    labelsTopLayout->addWidget(peakToPeakValueLabel);
    labelsTopLayout->addWidget(saturationIndicator);

    labelsBottomLayout->addWidget(varianceLabel);
    labelsBottomLayout->addWidget(stdDevLabel);
    labelsBottomLayout->addWidget(meanLabel);
    labelsBottomLayout->addWidget(medianLabel);

    mainLayout->addWidget(labelsContainer);

    connectSignalsAndSlots();

    setupTimer();

    storedTraces.clear();
    for (auto& series : storedSeries) {
        chart->removeSeries(series.get());
    }
    storedSeries.clear();
}

QString MainWindow::buttonStyle(const QString& color) {
    QString baseStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %1, stop:1 %2);
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 6px;
            font-weight: 600;
            font-size: 14px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %3, stop:1 %4);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %5, stop:1 %6);
        }
        QPushButton:disabled {
            background: #555555;
            color: #888888;
        }
    )";

    if (color == "green") {
        return baseStyle.arg("#4CAF50", "#45A049", "#45A049", "#3D8B40", R"(#3D8B40)", "#357935");
    } else if (color == "red") {
        return baseStyle.arg("#F44336", "#E53935", "#E53935", "#D32F2F", "#D32F2F", "#C62828");
    } else {
        return baseStyle.arg("#2196F3", "#1E88E5", "#1E88E5", "#1976D2", "#1976D2", "#1565C0");
    }

}

void MainWindow::setupButton(QPushButton* button, const QString& iconPath, const QString& tooltip) {
    button->setIcon(QIcon(iconPath));
    button->setIconSize(QSize(32, 32));
    button->setFixedSize(48, 48);

    QString enhancedTooltip;
    if (tooltip == "Start") {
        enhancedTooltip = tr("Start Acquisition (Ctrl+R)");
    } else if (tooltip == "Stop") {
        enhancedTooltip = tr("Stop Acquisition (Ctrl+S)");
    } else if (tooltip == "Save Data") {
        enhancedTooltip = tr("Save Data (Ctrl+D)");
    } else if (tooltip == "Toggle Average View") {
        enhancedTooltip = tr("Toggle Average View (Ctrl+A)");
    } else if (tooltip == "Set As Background") {
        enhancedTooltip = tr("Set As Background (Ctrl+B)");
    } else if (tooltip == "Store Current Trace") {
        enhancedTooltip = tr("Store Current Trace (Ctrl+T)");
    } else {
        enhancedTooltip = tooltip;
    }

    button->setToolTip(enhancedTooltip);

    button->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(255, 255, 255, 0.1);
            border: none;
            border-radius: 24px;
        }
        QPushButton:hover {
            background-color: rgba(76, 175, 80, 0.2);
        }
        QPushButton:pressed {
            background-color: rgba(76, 175, 80, 0.3);
        }
        QToolTip {
            background-color: #2C2C2C;
            color: white;
            border: 1px solid #555555;
            padding: 5px;
        }
    )");
}

QLabel* MainWindow::createStylishLabel(const QString& text) {
    auto label = new QLabel(text, this);
    label->setStyleSheet(R"(
        color: #DDDDDD;
        font-weight: 500;
        font-size: 14px;
        background-color: rgba(255, 255, 255, 0.05);
        padding: 10px;
        border-radius: 6px;
    )");
    return label;
}

void MainWindow::connectSignalsAndSlots() {
    bool connectionSuccessful = connect(setExposureButton, &QPushButton::clicked, this,
                                        &MainWindow::onSetExposureClicked);
    if (!connectionSuccessful) {
        qWarning() << "Failed to connect setExposureButton clicked signal.";
    }

    connectionSuccessful = connect(startButton, &QPushButton::clicked, this, &MainWindow::startDataAcquisition);
    if (!connectionSuccessful) {
        qWarning() << "Failed to connect startButton clicked signal.";
    }

    connectionSuccessful = connect(stopButton, &QPushButton::clicked, this, &MainWindow::stopDataAcquisition);
    if (!connectionSuccessful) {
        qWarning() << "Failed to connect stopButton clicked signal.";
    }

    connectionSuccessful = connect(saveDataButton, &QPushButton::clicked, this, &MainWindow::onSaveDataClicked);
    if (!connectionSuccessful) {
        qWarning() << "Failed to connect saveDataButton clicked signal.";
    }

    connectionSuccessful = connect(setBackgroundButton, &QPushButton::clicked, this, &MainWindow::onSetBackgroundClicked);
    if (!connectionSuccessful) {
        qWarning() << "Failed to connect setBackgroundButton clicked signal.";
    }

    connectionSuccessful = connect(showSubtractedValuesButton, &QPushButton::clicked, this, &MainWindow::onToggleSubtractedValuesView);
    if (!connectionSuccessful) {
        qWarning() << "Failed to connect showSubtractedValuesButton clicked signal.";
    }

    connectionSuccessful = connect(setRangeButton, &QPushButton::clicked, this, &MainWindow::onSetRangeClicked);
    if (!connectionSuccessful) {
        qWarning() << "Failed to connect setRangeButton clicked signal.";
    }

    connectionSuccessful = connect(showAverageButton, &QPushButton::clicked, this, &MainWindow::onToggleAverageView);
    if (!connectionSuccessful) {
        qWarning() << "Failed to connect showAverageButton clicked signal.";
    }
    connectionSuccessful = connect(storeTraceButton, &QPushButton::clicked, this, &MainWindow::onStoreTraceClicked);
    if (!connectionSuccessful) {
        qWarning() << "Failed to connect storeTraceButton clicked signal.";
    }

    connectionSuccessful = connect(toggleYRangeButton, &QPushButton::clicked, this, &MainWindow::onToggleYRangeClicked);
    if (!connectionSuccessful) {
        qWarning() << "Failed to connect toggleYRangeButton clicked signal.";
    }
}


void MainWindow::setupTimer() {
    if (timer) {
        // Disconnect the existing timer if already connected
        bool disconnectSuccessful = disconnect(timer, &QTimer::timeout, this, &MainWindow::updatePlot);
        if (!disconnectSuccessful) {
            qWarning() << "Failed to disconnect timer timeout signal.";
        }
    }

    // Create a new timer and set its interval
    timer = new QTimer(this);
    timer->setInterval(10);

    // Connect the timer's timeout signal to the updatePlot slot
    bool connectSuccessful = connect(timer, &QTimer::timeout, this, &MainWindow::updatePlot);
    if (!connectSuccessful) {
        qWarning() << "Failed to connect timer timeout signal.";
    }
}




void MainWindow::startDataAcquisition() {
    if (ftHandle == nullptr || fthandle_uart == nullptr) {
        updateStatusBar(tr("Device Error: Not properly initialized"), 5000);
        QMessageBox::critical(this, "Device Error", "Devices are not properly initialized. Please check the connection.");
        return;
    }

    updateStatusBar(tr("Starting data acquisition..."));

    // Clear the frame buffer
    frameBuffer.clear();

    // Clear stored traces
    storedTraces.clear();
    for (auto& series : storedSeries) {
        chart->removeSeries(series.data());
    }
    storedSeries.clear();

    // Clear the data buffer
    dataBuffer.clear();

    // Purge any existing data in the reception buffer
    FT_STATUS purgeStatus = FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);
    if (purgeStatus != FT_OK) {
        qDebug() << "Failed to purge buffers. Status:" << purgeStatus;
        updateStatusBar(tr("Warning: Buffer purge failed. Data may be inconsistent."), 5000);
        QMessageBox::warning(this, "Warning", "Failed to purge device buffers. Data may be inconsistent.");
    }

    // Set the exposure time
    try {
        updateStatusBar(tr("Setting exposure time..."));
        set_exp(defaultExposureTime);
    } catch (const std::exception& e) {
        qDebug() << "Failed to set exposure time:" << e.what();
        updateStatusBar(tr("Error: Failed to set exposure time"), 5000);
        QMessageBox::critical(this, "Error", QString("Failed to set exposure time: %1").arg(e.what()));
        return;
    }

    QThread::msleep(100);

    // Turn on the trigger
    try {
        updateStatusBar(tr("Enabling trigger..."));
        trig_on();
    } catch (const std::exception& e) {
        qDebug() << "Failed to turn on trigger:" << e.what();
        updateStatusBar(tr("Error: Failed to enable trigger"), 5000);
        QMessageBox::critical(this, "Error", QString("Failed to turn on trigger: %1").arg(e.what()));
        return;
    }

    // Start recording frames
    startRecording();

    // Start the timer for continuous data acquisition
    timer->start(10);

    // Update UI state
    startButton->setEnabled(false);
    stopButton->setEnabled(true);

    // Reset Y-axis if in auto range mode
    if (isAutoYRange) {
        auto axisY = dynamic_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
        if (axisY) {
            axisY->setRange(0, 65535);
        }
    }

    // Clear last ten frames buffer
    lastTenFrames.clear();

    qDebug() << "Continuous data acquisition started";
    updateStatusBar(tr("Data acquisition started successfully - Exposure: %1 μs").arg(defaultExposureTime));
}



void MainWindow::stopDataAcquisition() {
    qDebug() << "Stopping data acquisition...";

    // Stop the timer
    timer->stop();
    qDebug() << "Timer stopped";

    try {
        // Turn off the trigger
        trig_off();
        qDebug() << "Trigger turned off successfully";

        // Purge any remaining data in the reception and transmit buffers
        FT_STATUS purgeStatus = FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);
        if (purgeStatus != FT_OK) {
            qDebug() << "FT_Purge failed with status:" << purgeStatus;
            throw std::runtime_error("Failed to purge device buffers");
        }
        qDebug() << "Device buffers purged successfully";

        // Reset the device
        FT_STATUS resetStatus = FT_ResetDevice(ftHandle);
        if (resetStatus != FT_OK) {
            qDebug() << "FT_ResetDevice failed with status:" << resetStatus;
            throw std::runtime_error("Failed to reset device");
        }
        qDebug() << "Device reset successfully";

        // Clear internal buffers
        frameBuffer.clear();
        dataBuffer.clear();

        // Stop recording but keep the data in allFramesData
        isRecording = false;

        // Do not clear the current series or stored traces
        // This keeps the signal on the screen

        startButton->setEnabled(true);
        stopButton->setEnabled(false);

        // Do not reset labels or saturation indicator
        // This keeps the last values visible

        qDebug() << "Data acquisition stopped successfully";
    } catch (const std::exception& e) {
        qDebug() << "Exception caught while stopping data acquisition:" << e.what();
        QMessageBox::critical(this, "Error", QString("Failed to stop data acquisition: %1").arg(e.what()));
    }
}


void MainWindow::updatePlot() {
   if (ftHandle == nullptr || fthandle_uart == nullptr) {
       QMessageBox::critical(this, "Device Error", "Devices are not properly initialized. Please check the connection.");
       stopDataAcquisition();
       return;
   }

   constexpr int expectedFrameSize = 2088;

   // Read available data
   DWORD bytesAvailable;
   FT_GetQueueStatus(ftHandle, &bytesAvailable);

   if (bytesAvailable > 0) {
       QByteArray newData(bytesAvailable, 0);
       DWORD bytesRead;
       FT_STATUS status = FT_Read(ftHandle, newData.data(), bytesAvailable, &bytesRead);
       if (status != FT_OK) {
           qWarning() << "FT_Read failed with status:" << status;
           return;
       }
       if (bytesRead > 0) {
           dataBuffer.append(newData.left(bytesRead));
       }
   }

   int maxFramesToProcess = 10;

   while (dataBuffer.size() >= expectedFrameSize && maxFramesToProcess > 0) {
       int frameStart = findFrameStart(dataBuffer);
       if (frameStart == -1) {
           dataBuffer.remove(0, 4);
           break;
       }

       if (frameStart > 0) {
           dataBuffer.remove(0, frameStart);
           continue;
       }

       if (dataBuffer.size() < expectedFrameSize) {
           break;
       }

       QByteArray frameData = dataBuffer.left(expectedFrameSize);
       dataBuffer.remove(0, expectedFrameSize);

       QVector<QPointF> newPoints = processFrame(frameData);

       if (lastTenFrames.size() >= 10) {
           lastTenFrames.removeFirst();
       }
       lastTenFrames.append(newPoints);

       if (!showingAverage) {
           QMetaObject::invokeMethod(this, [this, newPoints]() {
               updatePlotWithPoints(newPoints);
           }, Qt::QueuedConnection);
       }

       maxFramesToProcess--;
   }

   // If buffer gets too large, clear old data
   if (dataBuffer.size() > expectedFrameSize * 100) {
       dataBuffer = dataBuffer.right(expectedFrameSize * 50);
   }

   if (showingAverage) {
       updateAveragePlot();
   }
}

int MainWindow::findFrameStart(const QByteArray& data) {
    if (data.size() < 4) return -1;

    for (int i = 0; i <= data.size() - 4; ++i) {
        if (static_cast<uint8_t>(data[i]) == 0x00 &&
            static_cast<uint8_t>(data[i + 1]) == 0x00 &&
            static_cast<uint8_t>(data[i + 2]) == 0x00 &&
            static_cast<uint8_t>(data[i + 3]) == 0x01) {
            return i;
            }
    }
    return -1;
}

void MainWindow::updatePlotWithPoints(const QVector<QPointF>& points) const {
    QVector<QPointF> filteredPoints = filterPointsByRange(points);
    updateMainSeries(filteredPoints);
    updatePeakIndicator(filteredPoints);
    updateAxisRanges(filteredPoints);
    updateLabels(filteredPoints);
}

QVector<QPointF> MainWindow::filterPointsByRange(const QVector<QPointF>& points) const {
    QVector<QPointF> filteredPoints;
    for (const auto& point : points) {
        if (point.x() >= currentMinRange && point.x() <= currentMaxRange) {
            filteredPoints.append(point);
        }
    }
    return filteredPoints;
}

void MainWindow::updateMainSeries(const QVector<QPointF>& filteredPoints) const {
    series->replace(filteredPoints);
}

MainWindow::Statistics MainWindow::calculateStatistics(const QVector<QPointF>& points) const {
    Statistics stats{};

    // First filter points to only include those within the current range
    QVector<QPointF> rangePoints;
    for (const auto& point : points) {
        if (point.x() >= currentMinRange && point.x() <= currentMaxRange) {
            rangePoints.append(point);
        }
    }

    if (rangePoints.isEmpty()) {
        return stats;
    }

    // Calculate mean
    double sum = 0.0;
    QVector<double> values;
    values.reserve(rangePoints.size());

    for (const auto& point : rangePoints) {  // Use filtered points
        sum += point.y();
        values.append(point.y());
    }
    stats.mean = sum / rangePoints.size();

    // Calculate variance and standard deviation
    double squaredSum = 0.0;
    for (const auto& value : values) {
        double diff = value - stats.mean;
        squaredSum += diff * diff;
    }
    stats.variance = squaredSum / rangePoints.size();
    stats.stdDev = std::sqrt(stats.variance);

    // Calculate median
    std::sort(values.begin(), values.end());
    if (values.size() % 2 == 0) {
        stats.median = (values[values.size()/2 - 1] + values[values.size()/2]) / 2.0;
    } else {
        stats.median = values[values.size()/2];
    }

    return stats;
}

void MainWindow::updatePeakIndicator(const QVector<QPointF>& filteredPoints) const {
    double peakValue = 0;
    int peakPixel = 0;
    double minValue = std::numeric_limits<double>::max();
    double maxValue = std::numeric_limits<double>::lowest();

    for (const auto& point : filteredPoints) {
        if (point.y() > peakValue) {
            peakValue = point.y();
            peakPixel = static_cast<int>(point.x());
        }
        if (point.y() < minValue) minValue = point.y();
        if (point.y() > maxValue) maxValue = point.y();
    }

    QVector<QPointF> peakPoints;
    if (peakPixel >= currentMinRange && peakPixel <= currentMaxRange) {
        double arrowHeight = (maxValue - minValue) * 0.1;
        double arrowWidth = (currentMaxRange - currentMinRange) * 0.02;

        peakPoints << QPointF(peakPixel, maxValue + arrowHeight)
                   << QPointF(peakPixel, peakValue + arrowHeight * 0.2);

        peakPoints << QPointF(peakPixel - arrowWidth, peakValue + arrowHeight * 0.6)
                   << QPointF(peakPixel, peakValue)
                   << QPointF(peakPixel + arrowWidth, peakValue + arrowHeight * 0.6)
                   << QPointF(peakPixel, peakValue + arrowHeight * 0.2);
    }

    peakLineSeries->replace(peakPoints);

    QPen peakPen(Qt::red);
    peakPen.setWidth(2);
    peakLineSeries->setPen(peakPen);
    peakLineSeries->setBrush(QBrush(Qt::red));
}

void MainWindow::updateAxisRanges(const QVector<QPointF>& filteredPoints) const {
    auto axisX = dynamic_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
    auto axisY = dynamic_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
    if (axisX && axisY) {
        axisX->setRange(currentMinRange, currentMaxRange);

        if (isAutoYRange) {
            double minValue = std::numeric_limits<double>::max();
            double maxValue = std::numeric_limits<double>::lowest();
            for (const auto& point : filteredPoints) {
                if (point.y() < minValue) minValue = point.y();
                if (point.y() > maxValue) maxValue = point.y();
            }
            double yPadding = (maxValue - minValue) * 0.15;
            axisY->setRange(minValue - yPadding, maxValue + yPadding);
        } else {
            axisY->setRange(userMinYRange, userMaxYRange);
        }
    }
}

void MainWindow::updateLabels(const QVector<QPointF>& filteredPoints) const {
    double peakValue = 0;
    int peakPixel = 0;
    double minValue = std::numeric_limits<double>::max();
    double maxValue = std::numeric_limits<double>::lowest();

    for (const auto& point : filteredPoints) {
        if (point.y() > peakValue) {
            peakValue = point.y();
            peakPixel = static_cast<int>(point.x());
        }
        if (point.y() < minValue) minValue = point.y();
        if (point.y() > maxValue) maxValue = point.y();
    }

    double peakToPeakValue = maxValue - minValue;

    // Update existing labels
    peakValueLabel->setText(QString("Peak Value: %1").arg(peakValue));
    peakPixelLabel->setText(QString("Peak Pixel: %1").arg(peakPixel));
    peakToPeakValueLabel->setText(QString("Peak to Peak Value: %1").arg(peakToPeakValue));

    // Calculate and update statistical labels
    Statistics stats = calculateStatistics(filteredPoints);
    varianceLabel->setText(QString("Variance: %1").arg(stats.variance, 0, 'f', 2));
    stdDevLabel->setText(QString("Std Dev: %1").arg(stats.stdDev, 0, 'f', 2));
    meanLabel->setText(QString("Mean: %1").arg(stats.mean, 0, 'f', 2));
    medianLabel->setText(QString("Median: %1").arg(stats.median, 0, 'f', 2));

    qDebug() << "Frame processed, Exp:" << defaultExposureTime;
}

void MainWindow::updateAveragePlot() {
    if (lastTenFrames.isEmpty()) return;

    QVector<QPointF> averagePoints;
    qsizetype numPoints = lastTenFrames.first().size();
    averagePoints.reserve(numPoints);

    for (int i = 0; i < numPoints; ++i) {
        double sum = 0;
        for (const auto& frame : lastTenFrames) {
            sum += frame[i].y();
        }
        double average = sum / static_cast<double>(lastTenFrames.size());
        averagePoints.append(QPointF(lastTenFrames.first()[i].x(), average));
    }

    QMetaObject::invokeMethod(this, [this, averagePoints]() {
        updatePlotWithPoints(averagePoints);
    }, Qt::QueuedConnection);
}

QVector<QPointF> MainWindow::processFrame(const QByteArray& frameData) {
    QVector<QPointF> newPoints;
    newPoints.reserve(1024); // 1044 total pixels - 10 from each side

    double peakValue = 0;
    double minValue = std::numeric_limits<double>::max();
    double maxValue = std::numeric_limits<double>::lowest();
    bool isSaturating = false;

    // Skip the 4-byte header and start processing data
    for (int i = 4; i < frameData.size(); i += 2) {
        uint16_t value = (static_cast<uint8_t>(frameData[i]) << 8) | static_cast<uint8_t>(frameData[i + 1]);
        auto adjustedValue = static_cast<double>(value);

        if (adjustedValue >= 65535) {
            isSaturating = true;
        }

        if (showSubtracted && !backgroundData.isEmpty() && (i / 2) - 2 < backgroundData.size()) {
            double backgroundValue = backgroundData.at((i / 2) - 2).y();
            adjustedValue = adjustedValue - backgroundValue;
        }

        // Adjust the pixel index to account for the header
        newPoints.append(QPointF(static_cast<double>((i - 4) / 2.0), adjustedValue));

        if (adjustedValue > peakValue) {
            peakValue = adjustedValue;
        }

        if (adjustedValue < minValue) minValue = adjustedValue;
        if (adjustedValue > maxValue) maxValue = adjustedValue;
    }

    if (isRecording) {
        allFramesData.append(newPoints);
    }

    // Update the saturation indicator
    QMetaObject::invokeMethod(this, [this, isSaturating]() {
        updateSaturationIndicator(isSaturating);
    }, Qt::QueuedConnection);

    return newPoints;
}

void MainWindow::onToggleAverageView() {
    showingAverage = !showingAverage;
    if (showingAverage) {
        updateAveragePlot();
    } else {
        updatePlot();
    }
}

void MainWindow::onToggleYRangeClicked() {
    isAutoYRange = toggleYRangeButton->isChecked();
    minYRangeSpinBox->setEnabled(!isAutoYRange);
    maxYRangeSpinBox->setEnabled(!isAutoYRange);

    if (isAutoYRange) {
        toggleYRangeButton->setText("Auto Y Range");
    } else {
        toggleYRangeButton->setText("Manual Y Range");
        userMinYRange = minYRangeSpinBox->value();
        userMaxYRange = maxYRangeSpinBox->value();
    }

    updatePlot();  // Update the plot to reflect the new Y-range settings
}













void MainWindow::trig_on() const {
    qDebug() << "Turning trigger on";
    char ex[4] = {2, 0, 0, 0};
    DWORD bytesWritten, bytesRead;
    int retries = 0;
    constexpr int maxRetries = 5;

    do {
        if (retries > 0) {
            qDebug() << "Retrying to turn trigger on. Attempt:" << retries + 1;
        }

        QThread::msleep(50);

        FT_STATUS writeStatus = FT_Write(fthandle_uart, ex, 4, &bytesWritten);
        if (writeStatus != FT_OK || bytesWritten != 4) {
            qDebug() << "Failed to write trig_on command. Status:" << writeStatus << "Bytes written:" << bytesWritten;
            throw std::runtime_error("Failed to write trig_on command");
        }

        FT_STATUS readStatus = FT_Read(fthandle_uart, ex, 1, &bytesRead);
        if (readStatus != FT_OK || bytesRead != 1) {
            qDebug() << "Failed to read trig_on response. Status:" << readStatus << "Bytes read:" << bytesRead;
            throw std::runtime_error("Failed to read trig_on response");
        }

        retries++;
    } while (ex[0] != 't' && retries < maxRetries);

    if (ex[0] != 't') {
        qDebug() << "Failed to turn trigger on after" << maxRetries << "attempts";
        throw std::runtime_error("Failed to turn trigger on");
    }

    qDebug() << "Trigger turned on successfully";
}

void MainWindow::trig_off() const {
    qDebug() << "Turning trigger off";
    char ex[4] = {3, 0, 0, 0};
    DWORD bytesWritten, bytesRead;
    int retries = 0;
    constexpr int maxRetries = 5;

    do {
        if (retries > 0) {
            qDebug() << "Retrying to turn trigger off. Attempt:" << retries + 1;
            QThread::msleep(50);
        }

        // Clear any pending data
        FT_Purge(fthandle_uart, FT_PURGE_RX | FT_PURGE_TX);

        // Write command
        FT_STATUS writeStatus = FT_Write(fthandle_uart, ex, 4, &bytesWritten);
        if (writeStatus != FT_OK || bytesWritten != 4) {
            qDebug() << "Failed to write trig_off command. Status:" << writeStatus << "Bytes written:" << bytesWritten;
            if (retries == maxRetries - 1) {
                throw std::runtime_error("Failed to write trig_off command");
            }
            continue;
        }

        // Wait for response
        QThread::msleep(20);

        // Read response
        FT_STATUS readStatus = FT_Read(fthandle_uart, ex, 1, &bytesRead);
        if (readStatus != FT_OK || bytesRead != 1) {
            qDebug() << "Failed to read trig_off response. Status:" << readStatus << "Bytes read:" << bytesRead;
            if (retries == maxRetries - 1) {
                throw std::runtime_error("Failed to read trig_off response");
            }
            continue;
        }

        qDebug() << "Received response byte:" << static_cast<int>(ex[0]);

        // Check for expected response ('t') or alternative valid responses
        if (ex[0] == 't' || ex[0] == 'T' || ex[0] == 0x74) {
            qDebug() << "Trigger turned off successfully";
            return;
        }

        retries++;
    } while (retries < maxRetries);

    qDebug() << "Failed to turn trigger off after" << maxRetries << "attempts. Last response:" << static_cast<int>(ex[0]);
    throw std::runtime_error("Failed to turn trigger off after maximum retries");
}


void MainWindow::set_exp(uint32_t exp) const {
    qDebug() << "set_exp called with exposure time:" << exp;

    char ex[1];
    int retries = 0;
    constexpr int maxRetries = 5;

    do {
        if (retries > 0) {
            qDebug() << "Retrying to set exposure time. Attempt:" << retries + 1;
        }

        QThread::msleep(50);

        DWORD bytesWritten;
        qDebug() << "Writing exposure time to device";
        FT_STATUS writeStatus = FT_Write(fthandle_uart, &exp, sizeof(exp), &bytesWritten);
        if (writeStatus != FT_OK) {
            qDebug() << "FT_Write failed with status:" << writeStatus;
            throw std::runtime_error("Failed to write exposure time to device");
        }

        if (bytesWritten != sizeof(exp)) {
            qDebug() << "Failed to write all bytes. Bytes written:" << bytesWritten;
            throw std::runtime_error("Failed to write all bytes of exposure time");
        }

        qDebug() << "Successfully wrote" << bytesWritten << "bytes to device";

        DWORD bytesRead;
        qDebug() << "Reading response from device";
        FT_STATUS readStatus = FT_Read(fthandle_uart, ex, 1, &bytesRead);
        if (readStatus != FT_OK) {
            qDebug() << "FT_Read failed with status:" << readStatus;
            throw std::runtime_error("Failed to read response from device");
        }

        if (bytesRead != 1) {
            qDebug() << "Failed to read response byte. Bytes read:" << bytesRead;
            throw std::runtime_error("Failed to read response byte");
        }

        qDebug() << "Read" << bytesRead << "bytes from device. Response:" << static_cast<int>(ex[0]);

        retries++;
    } while (ex[0] != 'A' && retries < maxRetries);

    if (ex[0] != 'A') {
        qDebug() << "Failed to set exposure time after" << maxRetries << "attempts";
        throw std::runtime_error("Device did not acknowledge exposure time change");
    }

    qDebug() << "Exposure time successfully set to:" << exp;
}



void MainWindow::onSetExposureClicked() {
    qDebug() << "onSetExposureClicked called";

    if (ftHandle == nullptr || fthandle_uart == nullptr) {
        qDebug() << "Device Error: Devices are not properly initialized.";
        QMessageBox::critical(this, "Device Error", "Devices are not properly initialized. Please check the connection.");
        return;
    }

    bool ok;
    int exposureTime = exposureTimeInput->text().toInt(&ok);
    if (!ok || exposureTime < 0) {
        qDebug() << "Invalid exposure time entered:" << exposureTimeInput->text();
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid non-negative exposure time.");
        return;
    }

    qDebug() << "Attempting to set exposure time to:" << exposureTime;

    const bool wasRunning = timer->isActive();
    if (wasRunning) {
        qDebug() << "Stopping data acquisition before changing exposure time";
        stopDataAcquisition();
    }

    qDebug() << "Data acquisition stopped, proceeding to set new exposure time";

    // Add a try-catch block to catch any exceptions
    try {
        qDebug() << "Calling set_exp with exposure time:" << exposureTime;
        set_exp(static_cast<uint32_t>(exposureTime));
        defaultExposureTime = exposureTime;
        qDebug() << "Exposure time successfully set to:" << exposureTime;
    } catch (const std::exception& e) {
        qDebug() << "Exception caught while setting exposure time:" << e.what();
        QMessageBox::critical(this, "Error", QString("Failed to set exposure time: %1").arg(e.what()));
        return;
    }

    if (wasRunning) {
        qDebug() << "Restarting data acquisition after changing exposure time";
        startDataAcquisition();
    }

    qDebug() << "onSetExposureClicked completed successfully";
}


void MainWindow::onSaveDataClicked() {
    QMessageBox msgBox;
    msgBox.setText("Choose save option:");
    QPushButton *allFramesButton = msgBox.addButton("All Frames", QMessageBox::ActionRole);
    QPushButton *lastFrameButton = msgBox.addButton("Last Frame", QMessageBox::ActionRole);
    QPushButton *chartImageButton = msgBox.addButton("Chart Image", QMessageBox::ActionRole);
    QPushButton *cancelButton = msgBox.addButton("Cancel", QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == cancelButton) {
        return;
    }

    if (msgBox.clickedButton() == chartImageButton) {
        saveChartImage();
        return;
    }

    bool saveAllFrames = (msgBox.clickedButton() == allFramesButton);
    bool saveLastFrame = (msgBox.clickedButton() == lastFrameButton);

    if (!saveAllFrames && !saveLastFrame) {
        return; // This shouldn't happen, but just in case
    }

    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Data File"),
                                                    QDir::homePath(),
                                                    tr("CSV Files (*.csv);;Text Files (*.txt);;JSON Files (*.json)"),
                                                    &selectedFilter);
    if (fileName.isEmpty()) return;

    QFileInfo fileInfo(fileName);
    QString extension = fileInfo.suffix().toLower();

    if (extension != "csv" && extension != "txt" && extension != "json") {
        // If no valid extension, default to the selected filter
        if (selectedFilter.contains("csv")) {
            fileName += ".csv";
            extension = "csv";
        } else if (selectedFilter.contains("txt")) {
            fileName += ".txt";
            extension = "txt";
        } else {
            fileName += ".json";
            extension = "json";
        }
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open file for writing."));
        return;
    }

    QTextStream out(&file);

    if (extension == "csv" || extension == "txt") {
        saveAsCSVorTXT(out, saveAllFrames, extension);
    } else if (extension == "json") {
        saveAsJSON(out, saveAllFrames);
    }

    file.close();

    QMessageBox::information(this, tr("Success"), tr("Data saved successfully."));
}




void MainWindow::saveAsCSVorTXT(QTextStream& out, bool saveAllFrames, const QString& extension) {
    QString separator = (extension == "csv") ? "," : "\t";
    Statistics stats = calculateStatistics(series->points());
    out << "Statistics:\n";
    out << "Mean" << separator << stats.mean << "\n";
    out << "Median" << separator << stats.median << "\n";
    out << "Variance" << separator << stats.variance << "\n";
    out << "Standard Deviation" << separator << stats.stdDev << "\n\n";
    // Write the current series data
    out << "Current Series Data:\n";
    out << "Pixel" << separator << "Intensity\n";
    for (const QPointF &point : series->points()) {
        out << point.x() << separator << point.y() << "\n";
    }

    // Write recorded frames
    if (!allFramesData.isEmpty()) {
        if (saveAllFrames) {
            out << "\nAll Recorded Frames:\n";
            out << "Frame" << separator << "Pixel" << separator << "Intensity\n";
            for (int frameIndex = 0; frameIndex < allFramesData.size(); ++frameIndex) {
                const auto& frame = allFramesData[frameIndex];
                for (const QPointF& point : frame) {
                    out << frameIndex << separator << point.x() << separator << point.y() << "\n";
                }
            }
        } else {
            out << "\nLast Recorded Frame:\n";
            out << "Pixel" << separator << "Intensity\n";
            const auto& lastFrame = allFramesData.last();
            for (const QPointF& point : lastFrame) {
                out << point.x() << separator << point.y() << "\n";
            }
        }
    }
}

void MainWindow::saveAsJSON(QTextStream& out, bool saveAllFrames) {
    QJsonObject rootObject;
    // Add statistics
    Statistics stats = calculateStatistics(series->points());
    QJsonObject statsObject;
    statsObject["mean"] = stats.mean;
    statsObject["median"] = stats.median;
    statsObject["variance"] = stats.variance;
    statsObject["standardDeviation"] = stats.stdDev;
    rootObject["statistics"] = statsObject;
    // Save current series data
    QJsonArray currentSeriesArray;
    for (const QPointF &point : series->points()) {
        QJsonObject pointObject;
        pointObject["pixel"] = point.x();
        pointObject["intensity"] = point.y();
        currentSeriesArray.append(pointObject);
    }
    rootObject["currentSeriesData"] = currentSeriesArray;

    // Save recorded frames
    if (!allFramesData.isEmpty()) {
        if (saveAllFrames) {
            QJsonArray allFramesArray;
            for (const auto & frame : allFramesData) {
                QJsonArray frameArray;
                for (const QPointF& point : frame) {
                    QJsonObject pointObject;
                    pointObject["pixel"] = point.x();
                    pointObject["intensity"] = point.y();
                    frameArray.append(pointObject);
                }
                allFramesArray.append(frameArray);
            }
            rootObject["allRecordedFrames"] = allFramesArray;
        } else {
            QJsonArray lastFrameArray;
            const auto& lastFrame = allFramesData.last();
            for (const QPointF& point : lastFrame) {
                QJsonObject pointObject;
                pointObject["pixel"] = point.x();
                pointObject["intensity"] = point.y();
                lastFrameArray.append(pointObject);
            }
            rootObject["lastRecordedFrame"] = lastFrameArray;
        }
    }

    QJsonDocument doc(rootObject);
    out << doc.toJson();
}


void MainWindow::onSetBackgroundClicked() {
    if (ftHandle == nullptr || fthandle_uart == nullptr) {
        QMessageBox::critical(this, "Device Error", "Devices are not properly initialized. Please check the connection.");
        return;
    }
    // Assuming 'series' is your QLineSeries with the current signal data
    backgroundData = series->points(); // Copy the current points to the background buffer
    if (backgroundData.size() > 1044) {
        backgroundData.resize(1044);
    }
}

void MainWindow::onToggleSubtractedValuesView() {
    if (ftHandle == nullptr || fthandle_uart == nullptr) {
        QMessageBox::critical(this, "Device Error", "Devices are not properly initialized. Please check the connection.");
        return;
    }
    // This flag tracks whether the subtracted values are currently being shown
    showSubtracted = !showSubtracted;

    // Update the plot with the new state
    updatePlot();
}



void MainWindow::showDiagnosticMessage(const QString& message, const QString& type) {
    QMessageBox::Icon icon;
    if (type == "Error") {
        icon = QMessageBox::Critical;
    } else if (type == "Warning") {
        icon = QMessageBox::Warning;
    } else {
        icon = QMessageBox::Information;
    }

    QMessageBox msgBox;
    msgBox.setIcon(icon);
    msgBox.setText(message);
    msgBox.setWindowTitle("Diagnostic Message");
    msgBox.exec();

    // Also log the message
    qDebug() << type << ": " << message;
}

void MainWindow::logError(const QString &message) {
    showDiagnosticMessage(message, "Error");
}

void MainWindow::startRecording() {
    allFramesData.clear();
    isRecording = true;
    qDebug() << "Started recording frames";
}

void MainWindow::stopRecording() {
    isRecording = false;
    qDebug() << "Stopped recording frames";
    saveAllFrames();
}

void MainWindow::saveAllFrames() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save All Frames"),
                                                    QDir::homePath(), tr("CSV Files (*.csv)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open file for writing."));
        return;
    }

    QTextStream out(&file);

    out << "Frame,Pixel,Intensity\n";

    for (int frameIndex = 0; frameIndex < allFramesData.size(); ++frameIndex) {
        const auto& frame = allFramesData[frameIndex];
        for (const QPointF& point : frame) {
            out << frameIndex << "," << point.x() << "," << point.y() << "\n";
        }
    }

    file.close();

    QMessageBox::information(this, tr("Success"), tr("All frames saved successfully."));
}


void MainWindow::onSetRangeClicked() {
    int newMinRange = minRangeSpinBox->value();
    int newMaxRange = maxRangeSpinBox->value();

    if (newMinRange >= newMaxRange) {
        QMessageBox::warning(this, "Invalid Range", "Min range must be less than max range.");
        return;
    }

    currentMinRange = newMinRange;
    currentMaxRange = newMaxRange;

    // Update the chart's x-axis range
    if (auto axisX = dynamic_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first())) {
        axisX->setRange(currentMinRange, currentMaxRange);
    }

    // Update all series with the new range
    updateAllSeriesWithNewRange();

    // Update the chart
    chart->update();
}

void MainWindow::updateSaturationIndicator(bool isSaturating) const {
    if (isSaturating) {
        saturationIndicator->setStyleSheet("background-color: red; border-radius: 20px;");
        saturationIndicator->setToolTip("Camera is saturating!");
    } else {
        saturationIndicator->setStyleSheet("background-color: green; border-radius: 20px;");
        saturationIndicator->setToolTip("Camera is operating normally");
    }
}

void MainWindow::onStoreTraceClicked() {
    if (series->points().isEmpty()) {
        QMessageBox::warning(this, "Warning", "No data to store.");
        return;
    }

    if (storedTraces.size() >= MAX_STORED_TRACES) {
        storedTraces.removeFirst();
        chart->removeSeries(storedSeries.first().data());
        storedSeries.removeFirst();
    }

    // Create a new series with only the points within the current range
    QVector<QPointF> rangeFilteredPoints;
    for (const auto& point : series->points()) {
        if (point.x() >= currentMinRange && point.x() <= currentMaxRange) {
            rangeFilteredPoints.append(point);
        }
    }

    storedTraces.append(rangeFilteredPoints);

    auto newSeries = QSharedPointer<QLineSeries>::create();
    newSeries->replace(rangeFilteredPoints);

    // Set a different color for each stored trace
    int hue = (static_cast<int>(storedTraces.size()) * 60) % 360;  // Use 60 degree intervals, wrap around at 360
    QColor traceColor = QColor::fromHsv(hue, 255, 255);
    newSeries->setColor(traceColor);
    newSeries->setOpacity(0.5);  // Make stored traces semi-transparent

    chart->addSeries(newSeries.data());
    newSeries->attachAxis(chart->axes(Qt::Horizontal).first());
    newSeries->attachAxis(chart->axes(Qt::Vertical).first());

    storedSeries.append(newSeries);

    // Update the chart
    chart->update();
}

void MainWindow::updateAllSeriesWithNewRange() {
    // Update main series
    QVector<QPointF> filteredPoints;
    for (const auto& point : series->points()) {
        if (point.x() >= currentMinRange && point.x() <= currentMaxRange) {
            filteredPoints.append(point);
        }
    }
    series->replace(filteredPoints);

    // Update stored traces
    for (int i = 0; i < storedTraces.size(); ++i) {
        QVector<QPointF> filteredStoredPoints;
        for (const auto& point : storedTraces[i]) {
            if (point.x() >= currentMinRange && point.x() <= currentMaxRange) {
                filteredStoredPoints.append(point);
            }
        }
        storedSeries[i]->replace(filteredStoredPoints);
    }

    // Update peak indicator
    updatePlotWithPoints(filteredPoints);
}

void MainWindow::saveChartImage() {
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Chart Image"),
                                                    QDir::homePath(),
                                                    tr("PNG Files (*.png);;SVG Files (*.svg)"),
                                                    &selectedFilter);
    if (fileName.isEmpty()) return;

    QFileInfo fileInfo(fileName);
    QString extension = fileInfo.suffix().toLower();

    if (extension != "png" && extension != "svg") {
        // If no valid extension, default to the selected filter
        if (selectedFilter.contains("png")) {
            fileName += ".png";
            extension = "png";
        } else {
            fileName += ".svg";
            extension = "svg";
        }
    }

    // Capture the entire window
    QPixmap pixmap = chartView->grab();

    bool success = false;

    if (extension == "png") {
        success = pixmap.save(fileName, "PNG");
    } else {
        QSvgGenerator generator;
        generator.setFileName(fileName);
        generator.setSize(pixmap.size());
        generator.setViewBox(pixmap.rect());
        generator.setTitle("Spectrometer Chart");
        generator.setDescription("Chart exported from MDSpectra application");

        QPainter painter(&generator);
        painter.drawPixmap(0, 0, pixmap);
        success = true; // SVG generation doesn't return a success status, so we assume it's successful
    }

    if (success) {
        QMessageBox::information(this, tr("Success"), tr("Chart image saved successfully."));
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to save chart image."));
    }
}

void MainWindow::setupShortcuts() {
    startShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this);
    stopShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this);
    saveShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_D), this);
    toggleAverageShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_A), this);
    setBackgroundShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_B), this);
    storeTraceShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_T), this);

    connect(startShortcut, &QShortcut::activated, this, &MainWindow::startDataAcquisition);
    connect(stopShortcut, &QShortcut::activated, this, &MainWindow::stopDataAcquisition);
    connect(saveShortcut, &QShortcut::activated, this, &MainWindow::onSaveDataClicked);
    connect(toggleAverageShortcut, &QShortcut::activated, this, &MainWindow::onToggleAverageView);
    connect(setBackgroundShortcut, &QShortcut::activated, this, &MainWindow::onSetBackgroundClicked);
    connect(storeTraceShortcut, &QShortcut::activated, this, &MainWindow::onStoreTraceClicked);
}

void MainWindow::updateStatusBar(const QString& message, int timeout) {
    statusBar->showMessage(message, timeout);
}
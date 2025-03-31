# LaserSpectraVue

**LaserSpectraVue** is an open-source C++ application designed for real-time laser spectroscopy data acquisition and visualization. The graphical user interface is built using the **Qt 6 framework**, including the **QtCharts** module for plotting.

This software is bundled with the camera hardware and released under the GNU General Public License v3 (GPLv3).

---

## 🔧 Features

- Real-time plotting of spectral data (via QtCharts)
- Configurable exposure and acquisition settings
- Save and export measurements (CSV, JSON, TXT)
- UI designed using Qt Widgets and Qt Designer

---

## 📦 Requirements

- Qt 6.x (with QtCharts, QtWidgets, QtSvg)
- CMake >= 3.16
- C++17-compatible compiler (e.g., GCC, Clang, MinGW)

---

## 🛠️ Build Instructions

```bash
# Clone the repo
https://github.com/Archie1372/Laserspectravue.git

# Create and enter a build directory
mkdir build && cd build

# Configure and build
cmake ..
make
```

---

## 📜 License

This software is licensed under the **GNU General Public License v3.0 (GPLv3)**. See the `LICENSE` file for details.

> Note: This application uses the **Qt framework**, including the **QtCharts module**, which is licensed under GPLv3. Therefore, the entire application must be distributed under the same license.

---

## 📫 Contact

This project is maintained and bundled with its associated hardware systems. Support inquiries should be directed to your hardware vendor or via the GitHub Issues page.

---

## 💡 Acknowledgments

- Qt and the QtCharts module
- Open-source contributors and users





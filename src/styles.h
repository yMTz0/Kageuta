#pragma once

#include <QString>

namespace Styles {

inline const QString ACCENT = "#5cb8a5";
inline const QString ACCENT_HOVER = "#7dd4c3";
inline const QString ACCENT_DIM = "#3a7a6d";
inline const QString BG_PRIMARY = "#0a0a10";
inline const QString BG_SECONDARY = "#0e1218";
inline const QString BG_CARD = "#12181f";
inline const QString BG_HOVER = "#1a2530";
inline const QString BORDER = "#1e2a35";
inline const QString TEXT_PRIMARY = "#d8e0e0";
inline const QString TEXT_SECONDARY = "#6a7880";

inline const QString APP_STYLESHEET = QStringLiteral(R"(
    * {
        font-family: 'Segoe UI', 'Arial', sans-serif;
    }

    QMainWindow {
        background-color: %1;
    }

    QWidget#centralWidget {
        background-color: %1;
    }

    QWidget#sidebar {
        background-color: %2;
        border-right: 1px solid %5;
    }

    QWidget#contentArea {
        background-color: %1;
    }

    QWidget#animeCard {
        background-color: %3;
        border-radius: 10px;
        border: 1px solid %5;
    }

    QWidget#animeCard:hover {
        background-color: %4;
        border-color: %6;
    }

    QWidget#episodeCard {
        background-color: %3;
        border-radius: 8px;
        border: 1px solid %5;
    }

    QWidget#episodeCard:hover {
        background-color: %4;
        border-color: %6;
    }

    QLabel#headerTitle {
        color: %6;
        font-size: 22px;
        font-weight: bold;
        letter-spacing: 2px;
    }

    QLabel#sectionTitle {
        color: %7;
        font-size: 16px;
        font-weight: bold;
    }

    QLabel#animeTitle {
        color: %7;
        font-size: 13px;
        font-weight: bold;
    }

    QLabel#episodeTitle {
        color: %7;
        font-size: 12px;
    }

    QLabel#ratingLabel {
        color: %6;
        font-size: 11px;
        font-weight: bold;
    }

    QLabel#qualityBadge {
        background-color: %8;
        color: white;
        font-size: 10px;
        font-weight: bold;
        padding: 2px 8px;
        border-radius: 4px;
    }

    QLineEdit#searchInput {
        background-color: %3;
        border: 1px solid %5;
        border-radius: 20px;
        padding: 8px 16px;
        color: %7;
        font-size: 13px;
        selection-background-color: %6;
    }

    QLineEdit#searchInput:focus {
        border-color: %6;
    }

    QLineEdit#searchInput::placeholder {
        color: %8;
    }

    QPushButton#navButton {
        background-color: transparent;
        color: %8;
        border: none;
        border-radius: 8px;
        padding: 10px 16px;
        text-align: left;
        font-size: 13px;
        font-weight: 500;
    }

    QPushButton#navButton:hover {
        background-color: %4;
        color: %7;
    }

    QPushButton#navButton:checked {
        background-color: %8;
        color: white;
    }

    QPushButton#playButton {
        background-color: %6;
        color: white;
        border: none;
        border-radius: 20px;
        padding: 10px 30px;
        font-size: 14px;
        font-weight: bold;
    }

    QPushButton#playButton:hover {
        background-color: %7;
    }

    QPushButton#actionButton {
        background-color: %3;
        color: %7;
        border: 1px solid %5;
        border-radius: 8px;
        padding: 8px 16px;
        font-size: 12px;
    }

    QPushButton#actionButton:hover {
        background-color: %8;
        border-color: %6;
    }

    QPushButton#controlButton {
        background-color: transparent;
        color: %7;
        border: none;
        border-radius: 6px;
        padding: 8px;
        font-size: 16px;
    }

    QPushButton#controlButton:hover {
        background-color: %4;
    }

    QSlider#progressSlider::groove:horizontal {
        height: 4px;
        background: %4;
        border-radius: 2px;
    }

    QSlider#progressSlider::handle:horizontal {
        background: %6;
        width: 14px;
        height: 14px;
        margin: -5px 0;
        border-radius: 7px;
    }

    QSlider#progressSlider::sub-page:horizontal {
        background: %6;
        border-radius: 2px;
    }

    QSlider#volumeSlider::groove:horizontal {
        height: 3px;
        background: %4;
        border-radius: 2px;
    }

    QSlider#volumeSlider::handle:horizontal {
        background: %7;
        width: 10px;
        height: 10px;
        margin: -4px 0;
        border-radius: 5px;
    }

    QSlider#volumeSlider::sub-page:horizontal {
        background: %6;
        border-radius: 2px;
    }

    QComboBox#sourceCombo {
        background-color: %3;
        color: %7;
        border: 1px solid %5;
        border-radius: 6px;
        padding: 6px 12px;
        font-size: 12px;
        min-width: 120px;
    }

    QComboBox#sourceCombo:hover {
        border-color: %6;
    }

    QComboBox#sourceCombo::drop-down {
        border: none;
        width: 24px;
    }

    QComboBox QAbstractItemView {
        background-color: %3;
        color: %7;
        border: 1px solid %5;
        selection-background-color: %6;
        padding: 4px;
    }

    QScrollArea {
        border: none;
        background-color: transparent;
    }

    QScrollBar:vertical {
        background-color: %1;
        width: 8px;
        border-radius: 4px;
    }

    QScrollBar::handle:vertical {
        background-color: %5;
        border-radius: 4px;
        min-height: 30px;
    }

    QScrollBar::handle:vertical:hover {
        background-color: %6;
    }

    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
        height: 0px;
    }

    QScrollBar:horizontal {
        background-color: %1;
        height: 8px;
        border-radius: 4px;
    }

    QScrollBar::handle:horizontal {
        background-color: %5;
        border-radius: 4px;
        min-width: 30px;
    }

    QScrollBar::handle:horizontal:hover {
        background-color: %6;
    }

    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
        width: 0px;
    }

    QToolTip {
        background-color: %3;
        color: %7;
        border: 1px solid %6;
        border-radius: 4px;
        padding: 4px 8px;
        font-size: 11px;
    }
)")
.arg(BG_PRIMARY)      // %1
.arg(BG_SECONDARY)    // %2
.arg(BG_CARD)         // %3
.arg(BG_HOVER)        // %4
.arg(BORDER)          // %5
.arg(ACCENT)          // %6
.arg(TEXT_PRIMARY)     // %7
.arg(TEXT_SECONDARY); // %8

} // namespace Styles

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## プロジェクト概要: ESP32を用いた低温調理器の自作

### 機能仕様
- ESP32に各種センサーなどを接続
- ロータリーエンコーダにより時間と温度を設定
- 設定温度と現在温度を比較し、PID制御によりSSRのAC100Vのスイッチをオン／オフの制御
- 適切な温度を設定時間の間、保持
- 調理終了

### 使用機材
- ESP32開発ボード
- OLEDディスプレイ（I2C接続）
- DS18B20温度センサー（1-Wire接続）
- プッシュスイッチ付きのロータリーエンコーダ（クリック付き）
- Solid State Relay（SSR）

## 開発環境のセットアップ

### Arduino IDE設定
1. ESP32ボードサポートをインストール
2. 必要なライブラリ：
   - OneWire（DS18B20用）
   - DallasTemperature（DS18B20用）
   - Adafruit SSD1306（OLEDディスプレイ用）
   - PID_v1（PID制御用）

## プロジェクト構造（予定）

```
SC_ESP32/
├── SC_ESP32.ino        # メインプログラム
├── config.h            # ピン設定、定数定義
├── temperature.cpp/h   # 温度センサー制御
├── display.cpp/h       # OLEDディスプレイ制御
├── encoder.cpp/h       # ロータリーエンコーダ制御
├── pid_control.cpp/h   # PID制御
└── ssr_control.cpp/h   # SSR制御
```

## ピン接続（参考）

- DS18B20: GPIO 4（1-Wire、プルアップ抵抗必要）
- OLED SDA: GPIO 21
- OLED SCL: GPIO 22
- ロータリーエンコーダ A: GPIO 32
- ロータリーエンコーダ B: GPIO 33
- ロータリーエンコーダ SW: GPIO 25
- SSR制御: GPIO 26

## 開発時の注意事項

- AC100V制御を含むため、安全性を最優先に設計すること
- 温度センサーの精度と応答性を考慮したPIDパラメータの調整が必要
- OLEDディスプレイの更新頻度は適切に設定（ちらつき防止）
- ロータリーエンコーダのチャタリング対策を実装すること
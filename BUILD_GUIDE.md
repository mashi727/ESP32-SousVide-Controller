# ESP32 低温調理器 完全構築ガイド

このガイドでは、ハードウェアの部品調達から組み立て、ソフトウェアのコンパイル・書き込みまでを詳細に解説します。

## 目次

1. [必要な部品リスト](#1-必要な部品リスト)
2. [必要な工具](#2-必要な工具)
3. [ハードウェア製作](#3-ハードウェア製作)
4. [ソフトウェア環境構築](#4-ソフトウェア環境構築)
5. [コンパイルと書き込み](#5-コンパイルと書き込み)
6. [動作確認](#6-動作確認)
7. [トラブルシューティング](#7-トラブルシューティング)

---

## 1. 必要な部品リスト

### 電子部品

| 部品名 | 型番/仕様 | 数量 | 参考価格 | 購入先例 |
|-------|----------|-----|---------|---------|
| ESP32開発ボード | ESP32-DevKitC V4 | 1 | ¥1,500 | Amazon, 秋月電子 |
| 防水温度センサー | DS18B20 (ステンレスプローブ型) | 1 | ¥800 | Amazon, AliExpress |
| OLEDディスプレイ | SSD1306 128×64 I2C接続 | 1 | ¥600 | Amazon, 秋月電子 |
| ロータリーエンコーダー | EC11互換 プッシュスイッチ付き | 1 | ¥300 | 秋月電子 |
| ソリッドステートリレー | SSR-25DA (25A/AC100-240V) | 1 | ¥1,500 | Amazon |
| 抵抗 | 4.7kΩ 1/4W | 1 | ¥10 | 秋月電子 |
| 抵抗 | 10kΩ 1/4W | 2 | ¥20 | 秋月電子 |
| コンデンサ | 0.1μF セラミック | 2 | ¥20 | 秋月電子 |
| ターミナルブロック | 2極 5.08mmピッチ | 3 | ¥150 | 秋月電子 |
| ピンヘッダー | 2.54mmピッチ | 適量 | ¥100 | 秋月電子 |
| ジャンパーワイヤー | オス-メス、メス-メス | 各10本 | ¥300 | Amazon |

### 電源・配線材料

| 部品名 | 仕様 | 数量 | 参考価格 |
|-------|------|-----|---------|
| USB電源アダプター | 5V 2A以上 | 1 | ¥1,000 |
| USBケーブル | Micro-USB or USB Type-C | 1 | ¥500 |
| AC電源ケーブル | VCTFケーブル 0.75sq以上 | 2m | ¥500 |
| 圧着端子 | 絶縁被覆付き | 10個 | ¥200 |
| 熱収縮チューブ | 各種サイズ | 1セット | ¥500 |

### ケース・機構部品

| 部品名 | 仕様 | 数量 | 参考価格 |
|-------|------|-----|---------|
| 防水ケース | IP65以上 150×100×70mm程度 | 1 | ¥2,000 |
| ケーブルグランド | PG7 or PG9 | 3個 | ¥300 |
| 基板固定用スペーサー | M3×10mm | 4セット | ¥200 |
| ユニバーサル基板 | 72×47mm | 1 | ¥200 |

**合計予算目安**: 約¥10,000〜¥12,000

---

## 2. 必要な工具

### 基本工具
- はんだごて（温度調整機能付き推奨、30W以上）
- はんだ（鉛フリー、φ0.8mm程度）
- フラックス
- はんだ吸い取り線
- ニッパー
- ワイヤーストリッパー
- ピンセット
- テスター（マルチメーター）

### あると便利な工具
- ブレッドボード（試作用）
- オシロスコープ
- ヒートガン（熱収縮チューブ用）
- 圧着工具
- ドリル（ケース加工用）

---

## 3. ハードウェア製作

### 3.1 回路図

```
                    ESP32 DevKit
                    ┌─────────────┐
                    │             │
    DS18B20 ───────┤ GPIO4       │
         │          │             │
        4.7kΩ       │             │
         │          │             │
        VCC ────────┤ 3.3V        │
                    │             │
    OLED SDA ──────┤ GPIO21      │
    OLED SCL ──────┤ GPIO22      │
                    │             │
    Encoder A ─────┤ GPIO32      │
    Encoder B ─────┤ GPIO33      │
    Encoder SW ────┤ GPIO25      │
                    │             │
    SSR+ ──────────┤ GPIO26      │
    SSR- ──────────┤ GND         │
                    │             │
                    └─────────────┘
```

### 3.2 ブレッドボードでの試作

1. **ESP32の設置**
   - ブレッドボードの中央にESP32を配置
   - 電源レール（3.3V、5V、GND）を接続

2. **温度センサーの接続**
   ```
   DS18B20配線:
   - 赤線: 3.3V
   - 黒線: GND
   - 黄線: GPIO4（4.7kΩプルアップ抵抗を3.3Vへ）
   ```

3. **OLEDディスプレイの接続**
   ```
   OLED配線:
   - VCC: 3.3V
   - GND: GND
   - SDA: GPIO21
   - SCL: GPIO22
   ```

4. **ロータリーエンコーダーの接続**
   ```
   エンコーダー配線:
   - A相: GPIO32（10kΩプルアップ）
   - B相: GPIO33（10kΩプルアップ）
   - SW: GPIO25（内部プルアップ使用）
   - COM: GND
   ```

5. **SSRの接続**
   ```
   SSR配線:
   - 入力+: GPIO26
   - 入力-: GND
   - 出力側: AC100V回路へ（注意！）
   ```

### 3.3 ユニバーサル基板への実装

1. **部品配置計画**
   - ESP32用ピンソケット配置
   - 端子台の配置（電源入力、センサー接続、SSR出力）
   - プルアップ抵抗の配置

2. **はんだ付け手順**
   - ピンソケットから始める
   - 背の低い部品（抵抗）から実装
   - 端子台は最後に実装

3. **配線**
   - 電源ラインは太めの線材使用（AWG22程度）
   - 信号線は細い線材でOK（AWG26程度）
   - 配線は裏面で行い、整理して固定

### 3.4 防水ケースへの組み込み

1. **ケース加工**
   ```
   必要な穴:
   - OLEDディスプレイ用窓（カット後アクリル板で保護）
   - ロータリーエンコーダー用穴（φ7mm）
   - ケーブルグランド用穴×3（電源入力、温度センサー、AC出力）
   ```

2. **部品取り付け順序**
   1. ケーブルグランドの取り付け
   2. ロータリーエンコーダーの固定
   3. OLEDディスプレイの固定（両面テープ推奨）
   4. 基板をスペーサーで固定
   5. 配線の接続と整理

3. **AC100V配線の安全対策**
   - SSR出力側は必ず絶縁
   - 圧着端子使用で確実な接続
   - 熱収縮チューブで被覆
   - アース線の接続（金属ケースの場合）

### 3.5 配線チェックリスト

- [ ] 電源電圧の確認（3.3V、5V）
- [ ] 各GPIOピンの導通確認
- [ ] プルアップ抵抗の値確認
- [ ] SSR入力側のLED点灯確認
- [ ] AC配線の絶縁抵抗測定
- [ ] ケースのアース接続確認

---

## 4. ソフトウェア環境構築

### 4.1 PlatformIO環境構築（推奨）

#### Windows

1. **Visual Studio Codeインストール**
   ```bash
   # 公式サイトからダウンロード
   https://code.visualstudio.com/download
   ```

2. **PlatformIO拡張機能インストール**
   - VS Codeを起動
   - 拡張機能タブ（Ctrl+Shift+X）
   - "PlatformIO"を検索してインストール
   - VS Code再起動

3. **ドライバーインストール**
   - CP2102/CH340ドライバー（ESP32ボードに応じて）
   - [CP2102ドライバー](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
   - [CH340ドライバー](http://www.wch-ic.com/downloads/CH341SER_ZIP.html)

#### macOS

1. **Homebrewインストール（未インストールの場合）**
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

2. **VS CodeとPlatformIOインストール**
   ```bash
   brew install --cask visual-studio-code
   # VS Code起動後、拡張機能からPlatformIOインストール
   ```

3. **ドライバー**
   - macOS Big Sur以降は通常不要
   - 必要な場合は上記Windowsと同じリンクから

#### Linux (Ubuntu/Debian)

1. **依存パッケージインストール**
   ```bash
   sudo apt update
   sudo apt install python3 python3-pip git
   ```

2. **VS Codeインストール**
   ```bash
   # Snapを使用
   sudo snap install code --classic
   
   # または.debパッケージ
   wget -qO- https://packages.microsoft.com/keys/microsoft.asc | gpg --dearmor > packages.microsoft.gpg
   sudo install -o root -g root -m 644 packages.microsoft.gpg /etc/apt/trusted.gpg.d/
   sudo sh -c 'echo "deb [arch=amd64,arm64,armhf signed-by=/etc/apt/trusted.gpg.d/packages.microsoft.gpg] https://packages.microsoft.com/repos/code stable main" > /etc/apt/sources.list.d/vscode.list'
   sudo apt update
   sudo apt install code
   ```

3. **シリアルポートアクセス権限**
   ```bash
   sudo usermod -a -G dialout $USER
   # ログアウト・ログインが必要
   ```

### 4.2 Arduino IDE環境構築（代替手段）

1. **Arduino IDEインストール**
   - [公式サイト](https://www.arduino.cc/en/software)から2.x版をダウンロード

2. **ESP32ボードサポート追加**
   - ファイル → 環境設定
   - 追加のボードマネージャURLに以下を追加:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
   
3. **ボードマネージャーからESP32インストール**
   - ツール → ボード → ボードマネージャー
   - "esp32"を検索してインストール

4. **必要なライブラリインストール**
   - ツール → ライブラリを管理
   - 以下をインストール:
     - OneWire
     - DallasTemperature
     - Adafruit SSD1306
     - Adafruit GFX Library
     - ArduinoJson

---

## 5. コンパイルと書き込み

### 5.1 プロジェクトの取得

```bash
# GitHubからクローン
git clone https://github.com/mashi727/ESP32-SousVide-Controller.git
cd ESP32-SousVide-Controller
```

### 5.2 設定のカスタマイズ

1. **WiFi設定の変更**
   `include/Config.h`を編集:
   ```cpp
   #define WIFI_SSID "あなたのWiFi名"
   #define WIFI_PASSWORD "あなたのパスワード"
   ```

2. **ピン配置の確認**
   使用するESP32ボードに応じて調整:
   ```cpp
   #define ONE_WIRE_BUS 4      // 温度センサー
   #define SSR_PIN 26          // SSR制御
   // 他のピン定義も確認
   ```

### 5.3 PlatformIOでのコンパイルと書き込み

#### コマンドライン方式

```bash
# プロジェクトディレクトリで実行

# 1. 依存関係のインストール
pio lib install

# 2. ビルド
pio run

# 3. ESP32を接続してポート確認
pio device list
# 出力例: /dev/ttyUSB0 (Linux), COM3 (Windows), /dev/cu.usbserial-0001 (macOS)

# 4. platformio.iniでポート設定
# upload_port = /dev/ttyUSB0  # あなたのポートに変更

# 5. 書き込み
pio run -t upload

# 6. シリアルモニター起動
pio device monitor -b 115200
```

#### VS Code GUI方式

1. VS CodeでプロジェクトフォルダーLoad
2. PlatformIOアイコンをクリック（左サイドバー）
3. PROJECT TASKSから:
   - Build: コンパイル
   - Upload: 書き込み
   - Monitor: シリアルモニター

### 5.4 Arduino IDEでのコンパイルと書き込み

1. **プロジェクトを開く**
   - SC_ESP32.inoをダブルクリック

2. **ボード設定**
   - ツール → ボード → ESP32 Arduino → ESP32 Dev Module
   - ツール → Upload Speed → 921600
   - ツール → ポート → 該当するCOMポート選択

3. **コンパイルと書き込み**
   - 検証ボタン（✓）: コンパイルのみ
   - 書き込みボタン（→）: コンパイルと書き込み

4. **シリアルモニター**
   - ツール → シリアルモニタ
   - ボーレート: 115200

### 5.5 書き込み時のトラブルシューティング

#### "Connecting..."で止まる場合

**自動書き込みモード対応ボード:**
何度か試すか、USBケーブルを変更

**手動書き込みモード（古いボード）:**
1. BOOTボタンを押したまま
2. ENボタンを短く押す
3. BOOTボタンを離す
4. 書き込み開始

#### ポートが見つからない場合

**Windows:**
```cmd
# デバイスマネージャーで確認
# ポート(COMとLPT)にデバイスがあるか確認
```

**macOS:**
```bash
ls /dev/cu.*
# cu.usbserial-* or cu.SLAB_USBtoUART等を探す
```

**Linux:**
```bash
ls /dev/ttyUSB*
# または
ls /dev/ttyACM*

# 権限エラーの場合
sudo chmod 666 /dev/ttyUSB0
```

---

## 6. 動作確認

### 6.1 初回起動確認

1. **シリアルモニター確認**
   ```
   期待される出力:
   ESP32 Sous Vide Controller Starting...
   Found 1 temperature sensor(s)
   Sensor 0 Address: 28XXXXXXXXXX
   Encoder initialized
   SSR Control initialized
   State Machine initialized
   SPIFFS mounted successfully
   WiFi connected. IP: 192.168.x.x
   Web server started
   Initialization complete
   ```

2. **各モジュールの確認**

   **温度センサー:**
   - 現在温度が表示される
   - センサーを手で温めて変化確認

   **OLED表示:**
   - 起動画面表示
   - "READY"状態で待機

   **エンコーダー:**
   - 回転で数値変化
   - クリックで画面遷移

   **SSR:**
   - GPIO26のLED点灯確認（SSRにLED内蔵の場合）

### 6.2 基本動作テスト

1. **温度設定モード**
   - ボタンクリックで設定モード
   - エンコーダー回転で温度変更（20-95°C）
   - 0.5°C刻みで調整可能

2. **時間設定モード**
   - 温度設定後、自動遷移
   - 1分〜48時間の範囲で設定

3. **PID制御確認**
   - 水を入れた容器で試験
   - 設定温度への到達確認
   - 温度安定性の確認

### 6.3 WiFi機能確認

1. **接続確認**
   ```bash
   # PCから
   ping 192.168.x.x  # シリアルモニターで表示されたIP
   ```

2. **Webインターフェース**
   - ブラウザで`http://192.168.x.x`アクセス
   - 温度・時間表示確認
   - コントロールボタン動作確認

### 6.4 安全機能テスト

1. **センサー切断テスト**
   - 動作中にセンサーを抜く
   - エラー表示とヒーター停止確認

2. **過熱保護**
   - 最高温度設定で動作確認
   - 100°C到達前に停止することを確認

---

## 7. トラブルシューティング

### 7.1 ハードウェア関連

| 症状 | 原因 | 対処法 |
|-----|------|--------|
| 電源が入らない | USB給電不足 | 2A以上のアダプター使用 |
| 温度が-127°C表示 | センサー未接続 | 配線確認、プルアップ抵抗確認 |
| 温度が不安定 | ノイズ | センサーケーブルをAC配線から離す |
| ディスプレイ表示なし | I2C接続不良 | SDA/SCL配線確認、I2Cアドレス確認 |
| エンコーダー反応なし | 配線ミス | GPIO配置確認、プルアップ確認 |
| SSR動作しない | 制御電圧不足 | 3.3V出力確認、SSRの仕様確認 |

### 7.2 ソフトウェア関連

| エラーメッセージ | 原因 | 対処法 |
|----------------|------|--------|
| "SPIFFS mount failed" | 初回起動 | 正常、自動フォーマットされる |
| "WiFi connection failed" | SSID/パスワード誤り | Config.h確認 |
| "Sensor initialization failed" | ライブラリ不足 | 必要ライブラリ再インストール |
| コンパイルエラー | ライブラリバージョン | platformio.iniの指定バージョン使用 |

### 7.3 PID調整

温度が安定しない場合のPID調整指針:

1. **オーバーシュート大**
   - Kp（比例）を下げる
   - Kd（微分）を上げる

2. **到達が遅い**
   - Kp（比例）を上げる
   - Ki（積分）を少し上げる

3. **振動する**
   - 全体的にゲインを下げる
   - 特にKiを小さくする

推奨初期値:
```cpp
#define DEFAULT_KP 2.0
#define DEFAULT_KI 0.5
#define DEFAULT_KD 1.0
```

### 7.4 よくある質問

**Q: 他の温度センサーは使える？**
A: DS18B20互換品なら可能。防水型推奨。

**Q: AC200Vで使いたい**
A: SSRが対応していれば可能。配線に注意。

**Q: 複数の温度センサーを使いたい**
A: コード修正で対応可能。1-Wireは複数接続対応。

**Q: スマホアプリは？**
A: Webインターフェースがレスポンシブ対応。専用アプリは今後検討。

---

## 8. 次のステップ

### 機能拡張のアイデア

1. **MQTT対応**
   - Home Assistant連携
   - 遠隔監視強化

2. **レシピ機能**
   - 温度プロファイル保存
   - 自動調理プログラム

3. **マルチゾーン対応**
   - 複数ヒーター制御
   - ゾーン別温度管理

### コミュニティ

- [GitHub Issues](https://github.com/mashi727/ESP32-SousVide-Controller/issues): バグ報告・機能要望
- Pull Request歓迎: 改善提案をお待ちしています

---

## 更新履歴

- 2024-11-01: 初版作成
- 今後: ユーザーフィードバックに基づき更新予定

---

*安全第一で製作を楽しんでください！*
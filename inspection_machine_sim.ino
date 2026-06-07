// 検査機シミュレータ

// ピン定義
#define SENSOR_PIN  A0  // 光センサー（ワーク検知）
#define LED_OK      7   // 緑LED（良品）
#define LED_NG      8   // 赤LED（不良）
#define SENSOR_THRESHOLD 500  // センサーしきい値

// 公差設定
double tol_max = 10.05;  // 上限
double tol_min = 9.95;   // 下限

// 集計
int total   = 0;
int ok      = 0;
int ng      = 0;
int no_work = 0;

// NG種別カウント
int ng_dimension_plus  = 0;
int ng_dimension_minus = 0;
int ng_scratch         = 0;
int ng_transfer        = 0;

void setup() {
    Serial.begin(9600);
    pinMode(LED_OK, OUTPUT);
    pinMode(LED_NG, OUTPUT);

    Serial.println("=== 検査機シミュレータ ===");
    Serial.println("※画像検査自動機の動作をシミュレートします");
    Serial.println("");
    Serial.print("公差設定 → 上限(max):");
    Serial.print(tol_max);
    Serial.print(" 下限(min):");
    Serial.println(tol_min);
    Serial.println("計測値をシリアルモニタから入力してください");
    Serial.println("（例：10.02）");
    Serial.println("----------------------------");
}

void led_ok(void) {
    digitalWrite(LED_OK, HIGH);
    digitalWrite(LED_NG, LOW);
    delay(500);
    digitalWrite(LED_OK, LOW);
}

void led_ng(void) {
    digitalWrite(LED_OK, LOW);
    digitalWrite(LED_NG, HIGH);
    delay(500);
    digitalWrite(LED_NG, LOW);
}

void show_summary(void) {
    Serial.println("");
    Serial.println("========== 集計 ==========");
    Serial.print("総数　　　："); Serial.println(total);
    Serial.print("良品　　　："); Serial.println(ok);
    Serial.print("不良　　　："); Serial.println(ng);
    Serial.print("ワークなし："); Serial.println(no_work);

    if (ng > 0) {
        Serial.println("--- NG内訳 ---");
        if (ng_dimension_plus)  { Serial.print("寸法不良（プラス目）　："); Serial.println(ng_dimension_plus); }
        if (ng_dimension_minus) { Serial.print("寸法不良（マイナス目）："); Serial.println(ng_dimension_minus); }
        if (ng_scratch)         { Serial.print("傷不良　　　　　　　："); Serial.println(ng_scratch); }
        if (ng_transfer)        { Serial.print("搬送異常　　　　　　："); Serial.println(ng_transfer); }
    }

    if (total - no_work > 0) {
        float ng_rate = (float)ng / (total - no_work) * 100.0;
        Serial.print("不良率　　：");
        Serial.print(ng_rate);
        Serial.println("%");
    }
    Serial.println("===========================");
}

void loop() {
    // 計測値の入力待ち
    if (Serial.available() == 0) return;

    String input = Serial.readStringUntil('\n');
    input.trim();

    // 終了コマンド
    if (input == "end") {
        show_summary();
        while (1); // 停止
        return;
    }

    total++;
    Serial.println("");
    Serial.print("[ワーク "); Serial.print(total); Serial.println("]");

    // 供給部センサー
    Serial.print("  供給部　　：");
    int sensor_val = analogRead(SENSOR_PIN);
    if (sensor_val < SENSOR_THRESHOLD) {
        Serial.println("ワークなし（センサー未検知）");
        led_ng();
        no_work++;
        Serial.println("  次のワークの計測値を入力（終了はend）");
        return;
    }
    Serial.println("ワーク検知");

    // 搬送部
    Serial.println("  切出部　　：1個切り出し完了");
    Serial.println("  爪搬送　　：検査部へ移動中...");
    Serial.println("  検査部　　：位置確認OK");

    // 計測値表示
    double measured = input.toDouble();
    Serial.print("  計測値　　：");
    Serial.println(measured);

    // 寸法判定
    Serial.print("  寸法検査　：");
    if (measured > tol_max) {
        Serial.print("NG（プラス目）計測値:");
        Serial.print(measured); Serial.print(" 上限:"); Serial.println(tol_max);
        Serial.println("  不良排出　：NG → 寸法不良（プラス目）");
        led_ng();
        ng++;
        ng_dimension_plus++;
        Serial.println("  次のワークの計測値を入力（終了はend）");
        return;
    }
    if (measured < tol_min) {
        Serial.print("NG（マイナス目）計測値:");
        Serial.print(measured); Serial.print(" 下限:"); Serial.println(tol_min);
        Serial.println("  不良排出　：NG → 寸法不良（マイナス目）");
        led_ng();
        ng++;
        ng_dimension_minus++;
        Serial.println("  次のワークの計測値を入力（終了はend）");
        return;
    }
    Serial.print("OK（計測値:"); Serial.print(measured);
    Serial.print(" 範囲:"); Serial.print(tol_min);
    Serial.print("〜"); Serial.print(tol_max); Serial.println("）");

    // 画像検査（傷判定はランダム）
    Serial.println("  画像検査　：検査中...");
    int r = random(100);
    if (r < 5) {
        Serial.println("  不良排出　：NG → 傷不良");
        led_ng();
        ng++;
        ng_scratch++;
    } else {
        Serial.println("  良品排出　：OK");
        led_ok();
        ok++;
    }

    Serial.println("  次のワークの計測値を入力（終了はend）");
}

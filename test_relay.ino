int relay_r = 2;        // S8550 (PNP) 控制方向: LOW=吸合, HIGH=放開
int relay_l = 7;        // S8550 (PNP) 控制方向: LOW=吸合, HIGH=放開
int motor_r = 3;  // C1384 (NPN) 控制速度: PWM 0~255
int motor_l = 6;  // C1384 (NPN) 控制速度: PWM 0~255
int foward = 8;
int backward = 13;

void setup() {
  pinMode(relay_r, OUTPUT);
  pinMode(motor_r, OUTPUT);
  pinMode(relay_l, OUTPUT);
  pinMode(motor_l, OUTPUT);
  pinMode(foward,OUTPUT);
  pinMode(backward,OUTPUT);
    
  Serial.begin(9600);
  Serial.println("=== 速度控制測試開始 ===");
  
  // 初始化: 繼電器設為 HIGH (PNP是關閉的/前進檔)
  digitalWrite(relay_r, HIGH); 
  digitalWrite(relay_l, HIGH); 
  digitalWrite(foward,LOW);
  digitalWrite(backward,LOW);
  // 初始化: 速度設為 0 (停車)
  analogWrite(motor_r, 0);
  analogWrite(motor_l, 0);
}

void loop() {
  // ==========================================
  // 測試 1: 前進檔 (relay_r OFF) - 慢慢加速
  // ==========================================
  Serial.println("方向: 前進 (Forward) - 加速中...");
  digitalWrite(relay_r, HIGH); // 設定方向
  digitalWrite(relay_l, HIGH); // 設定方向
  digitalWrite(foward,HIGH);
  digitalWrite(backward,LOW);
  
  // 用迴圈讓速度從 0 加到 255
  for (int speed = 0; speed <= 255; speed += 5) {
    analogWrite(motor_r, speed); // 寫入速度
    analogWrite(motor_l, speed); // 寫入速度
    delay(50); // 等一下，讓你聽得出聲音變化
  }
  
  Serial.println("達到全速 (Full Speed)!");
  delay(1000); // 全速跑 1 秒

  // ==========================================
  // 測試 2: 慢慢減速到停
  // ==========================================
  Serial.println("減速中...");
  for (int speed = 255; speed >= 0; speed -= 5) {
    analogWrite(motor_r, speed);
    analogWrite(motor_l, speed);
    digitalWrite(foward,LOW);
    digitalWrite(backward,LOW);
    delay(50);
  }
  
  Serial.println("停車 (Stop)");
  analogWrite(motor_r, 0); // 確保歸零
  analogWrite(motor_l, 0); // 確保歸零
  delay(1000); // 停 1 秒

  // ==========================================
  // 測試 3: 倒車檔 (relay_r ON) - 簡單測試一下
  // ==========================================
  Serial.println("方向: 倒車 (Backward) - 半速運轉");
  digitalWrite(relay_r, LOW); // 切換繼電器 (PNP導通 -> 吸合)
  digitalWrite(relay_l, LOW); // 切換繼電器 (PNP導通 -> 吸合)
  digitalWrite(foward,LOW);
  digitalWrite(backward,HIGH);
  delay(500); // 給繼電器一點時間切換
  
  // analogWrite(motor_r, 150); // 直接開半速 (PWM 150)
  // delay(2000);
    for (int speed = 0; speed <= 255; speed += 5) {
    analogWrite(motor_r, speed); // 寫入速度
    analogWrite(motor_l, speed); // 寫入速度
    delay(50); // 等一下，讓你聽得出聲音變化
  }
  Serial.println("減速中...");
  for (int speed = 255; speed >= 0; speed -= 5) {
    analogWrite(motor_r, speed);
    analogWrite(motor_l, speed);
    delay(50);
  }
  
  // 停車，準備下一輪
  analogWrite(motor_r, 0);
  digitalWrite(relay_r, HIGH); // 切回前進檔預備
  digitalWrite(relay_l, HIGH); // 切回前進檔預備
  Serial.println("--- 循環結束 ---\n");
  delay(2000);
}
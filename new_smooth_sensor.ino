int relay_r = 2;  // S8550 (PNP) 控制方向: LOW=吸合, HIGH=放開
int relay_l = 7;  // S8550 (PNP) 控制方向: LOW=吸合, HIGH=放開
int motor_r = 3;  // C1384 (NPN) 控制速度: PWM 0~255
int motor_l = 6;  // C1384 (NPN) 控制速度: PWM 0~255
int foward = 8; // 綠LED
int backward = 12;// 紅LED
int turn_r = 9;
int turn_l = 13;

int trig_r = 5;    // 超音波感測器 Trig腳接 Arduino pin 5
int echo_r = 4;    //超音波感測器 Echo 腳接 Arduino pin 4
int trig_l = 10;   // 超音波感測器 Trig腳接 Arduino pin 11
int echo_l = 11;   //超音波感測器 Echo 腳接 Arduino pin 12

long dur_r, cm_r, dur_l, cm_l, cm ;  //宣告計算距離時，需要用到的兩個實數
int set_dur = 8; // set target distance
int tol = 3; // set tolerance
int turn_gap = 10; // set turning tolerance

// [修正 1] 速度設定調整
int spd_h = 160;   // 直走速度
int spd_m = 140;   // 差速修正速度
int spd_l = 100;
int spd_turn = 200; // [新增] 轉向專用大扭力，解決轉不動的問題

void setup() {
  Serial.begin (9600); 
  pinMode(relay_r, OUTPUT);
  pinMode(motor_r, OUTPUT);
  pinMode(relay_l, OUTPUT);
  pinMode(motor_l, OUTPUT);
  pinMode(foward,OUTPUT);
  pinMode(backward,OUTPUT);
  pinMode(turn_r,OUTPUT);
  pinMode(turn_l,OUTPUT);
  pinMode(trig_r, OUTPUT);      
  pinMode(echo_r, INPUT);       
  pinMode(trig_l, OUTPUT);      
  pinMode(echo_l, INPUT);       
}

// [修正 2] 輕量版中位數濾波 (從5次改3次，反應快一倍)
long readDistanceMID(int trigPin, int echoPin) {
  int values[3]; // 改成3次就夠了
  
  for (int i = 0; i < 3; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    long duration = pulseIn(echoPin, HIGH, 15000); // Timeout 再縮短一點 (15ms)
    long dist = (duration / 2) / 29.1;
    
    if (dist == 0) dist = 300; 
    values[i] = dist;
    delay(2); // 縮短間隔
  }

  // 排序 3 個數值
  if(values[0] > values[1]) { int t=values[0]; values[0]=values[1]; values[1]=t; }
  if(values[1] > values[2]) { int t=values[1]; values[1]=values[2]; values[2]=t; }
  if(values[0] > values[1]) { int t=values[0]; values[0]=values[1]; values[1]=t; }

  return values[1]; // 回傳中間值
}

// [新增] 安全停車函式 (避免瞬間反向電流)
void stopMotors() {
    analogWrite(motor_r, 0);
    analogWrite(motor_l, 0);
}

void loop() {
  // Sensor Input
  cm_r = readDistanceMID(trig_r,echo_r);
  delay(5); // 稍微給一點喘息時間
  cm_l = readDistanceMID(trig_l,echo_l);

  // Check Data
  Serial.print("R: "); Serial.print(cm_r);
  Serial.print(" | L: "); Serial.print(cm_l);
  Serial.print(" Diff:"); Serial.println(cm_l - cm_r);

  cm = (cm_r+cm_l)/2;

  // --- 邏輯判斷開始 ---

  // 1. Turn Left (Big Diff)
  if (cm_r - cm_l > turn_gap){
    stopMotors(); // 先停車，防止繼電器火花
    delay(20);    // 給一點時間消磁

    digitalWrite(turn_r,LOW);
    digitalWrite(turn_l,HIGH);

    digitalWrite(relay_r, HIGH);
    digitalWrite(relay_l, LOW);
    
    digitalWrite(foward,LOW);
    digitalWrite(backward,LOW);
    
    // [修正] 用 spd_turn (大扭力)
    analogWrite(motor_l, spd_turn);
    analogWrite(motor_r, spd_turn);
    delay(100); 
  }
  
  // 2. Turn Right (Big Diff)
  else if(cm_l - cm_r > turn_gap){
    stopMotors(); // 先停車
    delay(20);

    digitalWrite(turn_r,HIGH);
    digitalWrite(turn_l,LOW);
    
    digitalWrite(relay_r, LOW);
    digitalWrite(relay_l, HIGH);
    
    digitalWrite(foward,LOW);
    digitalWrite(backward,LOW);
    
    // [修正] 用 spd_turn (大扭力)
    analogWrite(motor_l, spd_turn);
    analogWrite(motor_r, spd_turn);
    delay(100);
  }
  
  // 3. Fine Angle & Distance (直走微調)
  else {
    digitalWrite(turn_l,LOW);
    digitalWrite(turn_r,LOW);
    
    // Forward
    if (cm > set_dur + tol){
      // 這裡如果原本是在後退，突然變前進，也最好停一下
      // 但為了流暢度，我們先假設這裡變化沒那麼劇烈
      
      digitalWrite(relay_r, HIGH);
      digitalWrite(relay_l, HIGH);
      digitalWrite(foward,HIGH);
      digitalWrite(backward,LOW);
      
      if(cm_r > cm_l + 4){
        analogWrite(motor_l, spd_m);
        analogWrite(motor_r, spd_h);
      }
      else if(cm_l > cm_r + 4){
        analogWrite(motor_l, spd_h);
        analogWrite(motor_r, spd_m);
      }
      else{
        analogWrite(motor_l, spd_h);
        analogWrite(motor_r, spd_h);
      }
    }
    // Backward
    else if(cm < set_dur - tol){
      digitalWrite(relay_r, LOW);
      digitalWrite(relay_l, LOW);
      digitalWrite(foward,LOW);
      digitalWrite(backward,HIGH);
      analogWrite(motor_l, spd_h);
      analogWrite(motor_r, spd_h);
    }      
    // Stop
    else{
      stopMotors();
      digitalWrite(foward, LOW);
      digitalWrite(backward, LOW);
    }
    
    delay(50); // 縮短主迴圈 delay，讓反應更靈敏
  }   
}
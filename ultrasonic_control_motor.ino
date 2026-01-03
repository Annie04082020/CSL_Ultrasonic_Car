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
int set_dur = 12; // set target distance
int tol = 3; // set tolerance
int turn_gap = 10; // set turning tolerance
int spd_h = 160;
int spd_m = 140;
int spd_l = 100;

void setup() {
  Serial.begin (9600); //Set Serial Monitor and Arduino data transprtation 9600 bps (Bits Per Second)
  pinMode(relay_r, OUTPUT);
  pinMode(motor_r, OUTPUT);
  pinMode(relay_l, OUTPUT);
  pinMode(motor_l, OUTPUT);
  pinMode(foward,OUTPUT);
  pinMode(backward,OUTPUT);
  pinMode(turn_r,OUTPUT);
  pinMode(turn_l,OUTPUT);
  pinMode(trig_r, OUTPUT);      //Arduino 對外啟動距離感測器Trig腳，射出超音波 
  pinMode(echo_r, INPUT);       //超音波被障礙物反射後，Arduino讀取感測器Echo腳的時間差
  pinMode(trig_l, OUTPUT);      //Arduino 對外啟動距離感測器Trig腳，射出超音波 
  pinMode(echo_l, INPUT);       //超音波被障礙物反射後，Arduino讀取感測器Echo腳的時間差
}
// long readDistanceAVG(int trigPin, int echoPin){
//   long total =0;
//   int readings =7;
//   for (int i=0;i<readings;i++){
//     digitalWrite(trigPin, LOW);
//     delayMicroseconds(5);
//     digitalWrite(trigPin, HIGH);
//     delayMicroseconds(10);
//     digitalWrite(trigPin, LOW);
//     pinMode(echoPin, INPUT);
//     long duration = pulseIn(echoPin, HIGH,30000);
//     long distance = (duration/2) / 29.1; 
//     if(distance > 300 || distance == 0) distance = total/(i+1); // 過濾極端值
//     total += distance;
//     delay(10);
//   }
//   return total/readings;
// }
long readDistanceMID(int trigPin, int echoPin) {
  int values[5]; // 儲存 5 次讀取結果
  
  // 1. 連續讀取 5 次
  for (int i = 0; i < 5; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    // 設定 timeout 避免卡死
    long duration = pulseIn(echoPin, HIGH, 20000); 
    long dist = (duration / 2) / 29.1;
    
    // 如果讀到 0 (代表沒收到回波，通常是太遠或散射)，視為最大距離
    if (dist == 0) dist = 300; 
    
    values[i] = dist;
    delay(5); // 稍微縮短間隔，加快反應
  }

  // 2. 簡單的排序 (泡沫排序法)，找出中位數
  // 目的：把 5 個數字由小排到大
  for (int i = 0; i < 4; i++) {
    for (int j = i + 1; j < 5; j++) {
      if (values[i] > values[j]) {
        int temp = values[i];
        values[i] = values[j];
        values[j] = temp;
      }
    }
  }

  // 3. 回傳中間那個值 (第 3 個，index 2)
  // 這就是「中位數」，可以完美過濾掉突然跳出來的 0 或 300
  return values[2];
}

void loop() {
  // Sensor Input
  cm_r =readDistanceMID(trig_r,echo_r);
  delay(15);
  cm_l =readDistanceMID(trig_l,echo_l);

  // Check Data
  Serial.print("R: "); Serial.print(cm_r);
  Serial.print(" | L: "); Serial.print(cm_l);
  Serial.print(" Diff:"); Serial.println(cm_l - cm_r);

  // Motor Feedback
  cm = (cm_r+cm_l)/2;
  // Turn Left (Big Diff)
  if (cm_r-cm_l>turn_gap){
    digitalWrite(turn_r,LOW);
    digitalWrite(turn_l,HIGH);

    digitalWrite(relay_r, HIGH);
    digitalWrite(relay_l, LOW);
    digitalWrite(foward,LOW);
    digitalWrite(backward,LOW);
    
    analogWrite(motor_l,spd_m);
    analogWrite(motor_r,spd_m);
    delay(100);
  }
  // Turn Right (Big Diff)
  else if(cm_l-cm_r>turn_gap){
    digitalWrite(turn_r,HIGH);
    digitalWrite(turn_l,LOW);
    
    digitalWrite(relay_r, LOW);
    digitalWrite(relay_l, HIGH);
    digitalWrite(foward,LOW);
    digitalWrite(backward,LOW);
    
    analogWrite(motor_l,spd_m);
    analogWrite(motor_r,spd_m);
    delay(100);
  }
  // Fine Angle
  else {
    digitalWrite(turn_l,LOW);
    digitalWrite(turn_r,LOW);
    // forward
    if (cm>set_dur+tol){
      digitalWrite(relay_r, HIGH);
      digitalWrite(relay_l, HIGH);
      digitalWrite(foward,HIGH);
      digitalWrite(backward,LOW);
      if(cm_r > cm_l+4){
        analogWrite(motor_l,spd_m);
        analogWrite(motor_r,spd_h);
      }
      else if(cm_l > cm_r+4){
        analogWrite(motor_l,spd_h);
        analogWrite(motor_r,spd_m);
      }
      else{
        analogWrite(motor_l,spd_h);
        analogWrite(motor_r,spd_h);
      }
    }
    // backward
    else if(cm<set_dur-tol){
      digitalWrite(relay_r, LOW);
      digitalWrite(relay_l, LOW);
      digitalWrite(foward,LOW);
      digitalWrite(backward,HIGH);
      analogWrite(motor_l,spd_h);
      analogWrite(motor_r,spd_h);
    }      
    // Stop
    else{
      analogWrite(motor_r, 0);
      analogWrite(motor_l, 0);
      digitalWrite(foward, LOW);
      digitalWrite(backward, LOW);
    }
    delay(50);  
  }   
}

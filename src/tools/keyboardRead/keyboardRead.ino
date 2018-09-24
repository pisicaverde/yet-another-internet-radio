void setup() {
  pinMode(A0, INPUT);

}

void loop() {
  Serial.begin(115200);
  int val = analogRead(A0);
  Serial.println(val);
  delay(100);
}

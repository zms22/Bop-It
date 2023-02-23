void setup() {
  pinMode(8, OUTPUT);
  pinMode(9, INPUT);

}

void loop() {
  while (digitalRead(9) == HIGH)
  {
    digitalWrite(8, HIGH); 
    delay(1000);           
    digitalWrite(8, LOW);  
    delay(1000);
  }            
}

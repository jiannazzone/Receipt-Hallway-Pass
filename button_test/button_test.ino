#define bathroomPin 16
#define classroomPin 14
#define cafeteriaPin 12
#define otherPin 13

byte bathroom;
byte classroom;
byte cafeteria;
byte other;

void setup() {
  Serial.begin(9600);
  pinMode(bathroomPin, INPUT_PULLUP);
  pinMode(classroomPin, INPUT_PULLUP);
  pinMode(cafeteriaPin, INPUT_PULLUP);
  pinMode(otherPin, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  bathroom = digitalRead(bathroomPin);
  classroom = digitalRead(classroomPin);
  cafeteria = digitalRead(cafeteriaPin);
  other = digitalRead(otherPin);

  if (bathroom == LOW) {
    Serial.println("bathroom");
    delay(2000);
  } else if (classroom == LOW) {
    Serial.println("classroom");
    delay(2000);
  } else if (cafeteria == LOW) {
    Serial.println("cafeteria");
    delay(2000);
  } else if (other == LOW){
    Serial.println("other");
    delay(2000);
  }

}

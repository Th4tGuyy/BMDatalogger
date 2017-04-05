//#####################################################
//#####################################################
//
//      THIS IS THE START SCREEN
//
//#####################################################
//#####################################################

void show_flash(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("     BM Devs");
  delay(1000);
  lcd.setCursor(0,0);
  if (Is20x04Screen) lcd.setCursor(0,1);
  lcd.print("   Datalogger");
  delay(1000);
  if (!Is20x04Screen) {
    lcd.setCursor(0,0);
    lcd.print("                ");
    lcd.setCursor(0,0);
  }
  else
    lcd.setCursor(0,2);
  lcd.print("     " + VersionStr);
  lcd.setCursor(0,1);
  if (Is20x04Screen) lcd.setCursor(0,3);
  lcd.print(" eCtune ISR V3");
  delay(2000);  
}

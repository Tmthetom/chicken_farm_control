#pragma region Inicializace a nastaven�

/* Knihovny */

#include <AM2320.h>  // https://github.com/hibikiledo/AM2320
#include <LiquidCrystal.h>

/* Nastaven� p�eddefinovan�ch hodnot */

int setTemperature = 38;  // Ide�ln� teplota
int setHumidity = 80;  // Ide�ln� vlhkost
int setDays = 23;  // Maxim�ln� po�et dn�, kter� m��e za��zen� b�et

/* Nastaven� maxim�ln�ch a minim�ln�ch hodnot */

#define setTemperatureMinimum 18  // Nejmen�� mo�n� teplota
#define setTemperatureMaximum 45  // Maxim�ln� mo�n� teplota

#define setHumidityMinimum 20  // Nejmen�� mo�n� vlhkost
#define setHumidityMaximum 85  // Maxim�ln� mo�n� vlhkost

#define setDaysMinimum 0  // Nelze m�t z�porn� po�et dn�, proto tedy 0
#define setDaysMaximum 48  // Milis() dok�e poskytnut maxim�ln� 49,71026961805556 dn�, pot� p�ete�e

/* Nastaven� pin� periferi� */

#define button 2  // Tla��tko pro ovl�d�n� menu
#define heater 13  // Topen� pro oh��v�n� kurn�ku
#define fan 10  // V�tr�k pro ochlazov�n� kurn�ku
#define ledEdit 11  // Kontroln� dioda, ukazuj�c� aktivn� editaci menu
#define ledFinished 12  // Kontroln� dioda, ukazuj�c� dokon�en� programu (p�esa�en� maxim�ln�ch dn�)

/* Nastaven� displeje */

LiquidCrystal lcd(A0, A1, A2, 5, A3, 4);  // Piny displeje
#define lcdColumns 20  // Po�et sloupc�
#define lcdRows 4  // Po�et ��dk�

/* Nastaven� menu displeje */

#define rowHeader 0  // Pozice hlavi�ky textu na displeji
#define rowOne 1  // Pozice prvn�ho ��dku na displeji
#define rowTwo 2  // Pozice druh�ho ��dku na displeji
#define rowThree 3  // Pozice t�et�ho ��dku na displeji
#define positionText 0  // Pozice za��tku textu ��dku
#define positionLineLeft 7  // Pozice lev� odd�lovac� ��ry
#define positionLineRight 13  // Pozice prav� odd�lovac� ��ry
#define positionActualValue 8  // Pozice nam��en� hodnoty
#define positionSetValue 15  // Pozice nastaven� hodnoty
#define positionUnit 12  // Pozice jednotky (celsia, procenta)

#pragma region Vlastn� znaky displeje
byte degreeDesign[8] = {
	B00000,
	B11000,
	B11011,
	B00100,
	B00100,
	B00100,
	B00100,
	B00011
};

byte lineDesign[8] = {
	B00100,
	B00100,
	B00100,
	B00100,
	B00100,
	B00100,
	B00100,
	B00100
};
#pragma endregion

/* Nastaven� encoderu pro nastavov�n� hodnot v menu (digit�ln�m potenciometrem) */

#define encoderPinA 9  // Pin encoderu A
#define encoderPinB 3  // Pin encoderu B

int encoderLastValue = LOW;  // Posledn� hodnota encoderu
int encoderCurrentValue = LOW;  // Aktu�ln� hodnota encoderu

/* Nastaven� hygrometru (teplota a vlkost) */

AM2320 hygrometer;

/* Nastaven� blik�n� pr�v� upravovan�ch hodnot */

#define blinkInterval 200  // Interval bliknut� v milisekund�ch

unsigned long previousMillis = 0;  // P�edchoz� po�et milisekund
unsigned long currentMillis;  // Aktu�ln� po�et milisekund

/* Nastaven� edita�n�ho menu */

#define menuNotSelected -1  // ��dn� menu nevybr�no
#define menuTemperature 0  // ��slo menu teploty
#define menuHumidity 1  // ��slo menu vlhkosti
#define menuDays 2  // ��slo menu po�tu dn�

int editedMenu = menuNotSelected;  // Aktu�ln� nastavovan� menu
int lastButtonState = LOW;  // Posledn� stav tla��tka
int currentButtonState;  // Aktu�ln� stav tla��tka
int lastDaysFromStart = 0;  // Statick� hodnota posledn�ho �asu, zobrazuj�c�ho se p�i editaci
char emptyValue[] = "  ";  // Pr�zdn� hodnota p�i blik�n�
bool isEditedMenuVisible = true;  // Je editovan� menu viditeln�?

#pragma endregion

#pragma region Hlavn� program - Vol�n� jednotliv�ch funkc�
// TODO: Displej se nep�ekresluje, pouze v�ci p�episujeme. Bylo by dobr� statick� texty vykreslit pouze jednou.

/* Nastaven� p�ed prvn�m spu�t�n�m */
void setup() {

	// Nastaven� periferi�
	pinMode(button, INPUT_PULLUP);  // Tla��tko pro ovl�d�n� menu
	pinMode(heater, OUTPUT);  // Topen� pro oh��v�n� kurn�ku
	pinMode(fan, OUTPUT);  // V�tr�k pro ochlazov�n� kurn�ku
	pinMode(ledEdit, OUTPUT);  // Kontroln� dioda, ukazuj�c� aktivn� editaci menu
	pinMode(ledFinished, OUTPUT);  // Kontroln� dioda, ukazuj�c� dokon�en� programu (p�esa�en� maxim�ln�ch dn�)

	// Nastaven� hygrometru
	hygrometer.begin();  // Inicializace I2C pro AM2320
	pinMode(encoderPinA, INPUT);  // Nastaven� pinu encoderu jako vstupu
	pinMode(encoderPinB, INPUT);  // Nastaven� pinu encoderu jako vstupu

								  // Nastaven� displeje
	lcd.begin(lcdColumns, lcdRows);  // Nastaven� rozli�en� displeje
	lcd.createChar(0, degreeDesign);  // Vytvo�en� nov�ho znaku displeje
	lcd.write(byte(0));  // P�id�n� nov�ho znaku do pam�ti displeje
	lcd.createChar(1, lineDesign);  // Vytvo�en� nov�ho znaku displeje
	lcd.write(byte(0));  // P�id�n� nov�ho znaku do pam�ti displeje
}

/* Hlavn� program */
void loop() {
	measure();  // Zm�� aktu�ln� hodnoty
	checkForMenuEdit();  // Pohyb v edita�n�m menu
	showMenu();  // Vykresl� menu
	control();  // Ovl�d�n� periferi� na z�klad� m��en�
}

#pragma endregion

#pragma region M��en� - Teploty a vlhkosti

/* Zm�� aktu�ln� hodnoty */
void measure() {

	// Pokud je menu editov�no, odejdi
	if (editedMenu != menuNotSelected) return;

	// M��en� aktu�ln�ch hodnot
	hygrometer.measure();  // Zm��� v�echny hodnoty, kter� se pak v kodu vy��taj�
}

#pragma endregion

#pragma region Editace - Pohyb v edita�n�m menu pomoc� tla��tka

/* Pohyb v edita�n�m menu */
void checkForMenuEdit() {
	// TODO: O�et�en� z�kmitu tla��tka (Debounce)

	// Na�ten� aktu�ln�ho stavu tla��tka
	currentButtonState = digitalRead(button);

	// Tla��tko zm��knut�
	if (currentButtonState == LOW && lastButtonState == HIGH) {
		switch (editedMenu) {

		// Editace teploty
		case menuNotSelected:  // Aktu�ln� needitujeme ��dn� menu
			editedMenu = menuTemperature;  // Jdeme editovat teplotu
			digitalWrite(ledEdit, HIGH);  // Zapnut� signaliza�n� diody editace
			break;

		// Editace vlhkosti
		case menuTemperature:  // Aktu�ln� editujeme teplotu
			editedMenu = menuHumidity;  // Jdeme editovat vlhkost
			break;

		// Editace dn�
		case menuHumidity:  // Aktu�ln� editujeme vlhkost
			editedMenu = menuDays;  // Jdeme editovat dny
			break;

		// Konec editace
		case menuDays:  // Aktu�ln� editujeme dny
			editedMenu = menuNotSelected;  // Kon��me s editac�
			isEditedMenuVisible = true;  // Pokud ukon��me editaci, upravovan� hodnoty jsou v�dy viditeln�
			digitalWrite(ledEdit, LOW);  // Vypnut� signaliza�n� diody editace
			break;
		}
	}

	// Aktualizace star�ho stavu
	lastButtonState = currentButtonState;
}

#pragma endregion

#pragma region Menu - Vykreslen� menu na displej

/* Vykreslen� menu */
void showMenu() {
	printDividingLines();  // Odd�lovac� ��ry
	printHeaderText();  // Hlavi�ka
	printTemperature();  // Teplota
	printHumidity();  // Vlhkost
	printDays();  // Dny
}

/* Vykresl� odd�lovac� ��ry */
void printDividingLines() {

	// Lev� ��ra
	lcdPrint(positionLineLeft, 0, (char)1);
	lcdPrint(positionLineLeft, 1, (char)1);
	lcdPrint(positionLineLeft, 2, (char)1);
	lcdPrint(positionLineLeft, 3, (char)1);

	// Prav� ��ra
	lcdPrint(positionLineRight, 0, (char)1);
	lcdPrint(positionLineRight, 1, (char)1);
	lcdPrint(positionLineRight, 2, (char)1);
	lcdPrint(positionLineRight, 3, (char)1);
}

/* Vykresl� hlavi�ku (ACT, SET) */
void printHeaderText() {
	lcdPrint(positionActualValue + 1, rowHeader, "ACT");
	lcdPrint(positionSetValue, rowHeader, "SET");
}

/* Vykresl� teplotu */
void printTemperature() {

	// Vykreslen� za��tku ��dku
	lcdPrint(positionText, rowOne, "TEMP");  // N�zev ��dku
	lcdPrint(positionActualValue, rowOne, readTemperature());  // Nam��en� hodnota
	lcdPrint(positionUnit, rowOne, (char)0);  // Znak stupn� celsia

	// P�ednastaven� hodnota
	if (editedMenu != menuTemperature) {  // Pokud toto menu nen� zrovna editov�no
		lcdPrint(positionSetValue, rowOne, setTemperature);  // P�ednastaven� hodnota
		return;  // Konec vykreslen�
	}

	// Blik�n� p�enastavovan� hodnoty
	if (isBlinkTime()) {

		// Zhasnut� hodnoty
		if (isEditedMenuVisible) {
			lcdPrint(positionSetValue, rowOne, emptyValue);  // Pr�zdn� hodnota 
			isEditedMenuVisible = false;
		}

		// Uk�zan� hodnoty
		else {
			lcdPrint(positionSetValue, rowOne, setTemperature);  // P�ednastaven� hodnota
			isEditedMenuVisible = true;
		}
	}

	// Nastavov�n� p�ednastaven� hodnoty pomoc� potenciometru
	setTemperature += readEncoder();

	// O�et�en� minim�ln� a maxim�ln� hodnoty
	if (setTemperature < setTemperatureMinimum) setTemperature = setTemperatureMinimum;  // Minim�ln� hodnota
	else if (setTemperature > setTemperatureMaximum) setTemperature = setTemperatureMaximum;  // Maxim�ln� hodnota
}

/* Vykresl� vlhkost */
void printHumidity() {

	// Vykreslen� za��tku ��dku
	lcdPrint(positionText, rowTwo, "HUMI");  // N�zev ��dku
	lcdPrint(positionActualValue, rowTwo, readHumidity());  // Nam��en� hodnota
	lcdPrint(positionUnit, rowTwo, "%");  // Znak procent

	// P�ednastaven� hodnota
	if (editedMenu != menuHumidity) {  // Pokud toto menu nen� zrovna editov�no
		lcdPrint(positionSetValue, rowTwo, setHumidity);  // P�ednastaven� hodnota
		return;  // Konec vykreslen�
	}

	// Blik�n� p�enastavovan� hodnoty
	if (isBlinkTime()) {

		// Zhasnut� hodnoty
		if (isEditedMenuVisible) {
			lcdPrint(positionSetValue, rowTwo, emptyValue);  // Pr�zdn� hodnota 
			isEditedMenuVisible = false;
		}

		// Uk�zan� hodnoty
		else {
			lcdPrint(positionSetValue, rowTwo, setHumidity);  // P�ednastaven� hodnota
			isEditedMenuVisible = true;
		}
	}

	// Nastavov�n� p�ednastaven� hodnoty pomoc� potenciometru
	setHumidity += readEncoder();

	// O�et�en� minim�ln� a maxim�ln� hodnoty
	if (setHumidity < setHumidityMinimum) setHumidity = setHumidityMinimum;  // Minim�ln� hodnota
	else if (setHumidity > setHumidityMaximum) setHumidity = setHumidityMaximum;  // Maxim�ln� hodnota
}

/* Vykresl� �as */
void printDays() {

	// N�zev ��dku
	lcdPrint(positionText, rowThree, "DAYS");

	// Po�etn� dn� od zapnut�
	if (editedMenu != menuDays) {  // Pokud toto menu nen� zrovna editov�no
		if (setDays < 10) lcdPrint(positionActualValue + 1, rowThree, getDaysFromStart());  // Jednotky
		else lcdPrint(positionActualValue, rowThree, getDaysFromStart());  // Des�tky
	}
	else {
		if (setDays < 10) lcdPrint(positionActualValue + 1, rowThree, lastDaysFromStart);  // Jednotky
		else lcdPrint(positionActualValue, rowThree, lastDaysFromStart);  // Des�tky
	}
	

	// Nastaven� po�et dn�
	if (editedMenu != menuDays) {  // Pokud toto menu nen� zrovna editov�no
		if (setDays < 10) lcdPrint(positionSetValue + 1, rowThree, setDays);  // Jednotky
		else lcdPrint(positionSetValue, rowThree, setDays);  // Des�tky
		return;  // Konec vykreslen�
	}

	// Blik�n� p�enastavovan� hodnoty
	if (isBlinkTime()) {

		// Zhasnut� hodnoty
		if (isEditedMenuVisible) {
			lcdPrint(positionSetValue, rowThree, emptyValue);  // Pr�zdn� hodnota 
			isEditedMenuVisible = false;
		}

		// Uk�zan� hodnoty
		else {
			if (setDays < 10) lcdPrint(positionSetValue + 1, rowThree, setDays);  // Jednotky
			else lcdPrint(positionSetValue, rowThree, setDays);  // Des�tky
			isEditedMenuVisible = true;
		}
	}

	// Nastavov�n� p�ednastaven� hodnoty pomoc� potenciometru
	setDays += readEncoder();

	// O�et�en� minim�ln� a maxim�ln� hodnoty
	if (setDays < setDaysMinimum) setDays = setDaysMinimum;  // Minim�ln� hodnota
	else if (setDays > setDaysMaximum) setDays = setDaysMaximum;  // Maxim�ln� hodnota
}

/* P�e�te a vr�t� hodnotu teplom��u */
float readTemperature() {
	return hygrometer.getTemperature();
}

/* P�e�te a vr�t� hodnotu vlhkom�ru */
float readHumidity() {
	return hygrometer.getHumidity();
}

/* Vr�t� po�et dn� od startu programu */
int getDaysFromStart() {
	// Jeden den m� 86 400 000 ms
	return lastDaysFromStart = millis() / 86400000;

	// Podrobn� verze
	/*
	return lastDaysFromStart = (  
		millis()  // Po�et ms od startu procesoru
		/ 1000  // P�evod na vte�iny (vte�ina m� 1000 ms)
		/ 60  // P�evod na minuty (minuta m� 60 vte�in)
		/ 60  // P�evod na hodiny (hodina m� 60 minut)
		/ 24  // P�evod na dny (den m� 24 hodin)
	);
	*/
}

/* P�e�te hodnotu z encoderu a vrac� hodnotu posunu */
/* 1 = Zv��en�, -1 = Zmen�en�, 0 = Bez zm�ny */
int readEncoder() {
	encoderCurrentValue = digitalRead(encoderPinA);
	if ((encoderLastValue == LOW) && (encoderCurrentValue == HIGH)) {
		encoderLastValue = encoderCurrentValue;
		if (digitalRead(encoderPinB) == LOW) return -1;
		else return 1;
	}
	encoderLastValue = encoderCurrentValue;
	return 0;
}

/* Je �as bliknout hodnotou v menu? */
bool isBlinkTime() {
	currentMillis = millis();

	if (currentMillis - previousMillis >= blinkInterval) {  // Nastal �as
		previousMillis = currentMillis;  // P�ep�eme �as posledn�ho blinkut�
		return true;  // Ozn�m�me �e je �as na bliknut�
	}

	return false;  // Ozn�m�me �e nen� �as na bliknut�
}

/* Vyps�n� textu na displej s ur�itou pozic� */
void lcdPrint(int column, int row, String text) {
	lcd.setCursor(column, row);
	lcd.print(text);
}
void lcdPrint(int column, int row, int value) {
	lcdPrint(column, row, String(value));
}
void lcdPrint(int column, int row, float value) {
	lcdPrint(column, row, String(value));
}
void lcdPrint(int column, int row, char value) {
	lcd.setCursor(column, row);
	lcd.print(value);
}

#pragma endregion

#pragma region V�stup - Ovl�d�n� periferi� na z�klad� m��en�

/* Ovl�d�n� periferi� na z�klad� m��en� */
void control() {

	// Reguluje
	if (getDaysFromStart() <= setDays) {

		// Topen�
		if (readTemperature() < setTemperature) digitalWrite(heater, HIGH);  // Top�
		else digitalWrite(heater, LOW);  // Netop�

		// V�tr�n�
		if (readHumidity() < setHumidity) digitalWrite(fan, HIGH);  // V�tr�
		else digitalWrite(fan, LOW);  // Nev�tr�

		// Pokud byla prodlou�ena �ivotnost programu, zv�t�en�m
		// maxim�ln�ho po�tu dn�, resetujeme signaliza�n� diodu
		if (digitalRead(ledFinished) == HIGH) digitalWrite(ledFinished, LOW);
	}

	// Nereguluje (po�et dn� p�esa�en - program ukon�en)
	else {
		digitalWrite(heater, LOW);  // Netop�
		digitalWrite(fan, LOW);  // Nev�tr�
		digitalWrite(ledFinished, HIGH);  // Zapnut� signalizace o ukon�en� programu
	}
}

#pragma endregion

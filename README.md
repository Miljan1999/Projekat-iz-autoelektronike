# Projekat iz autoelektronike
 Autoelektronika, projekat parking senzor
Uvod:

Zadatak projekta je napraviti (isprogramirati) i istestirati RTOS sistem parking senzora. Projekat je kreiran u VisualStudio2019 softveru. Tako?e, u projektu su implementirani i MISRA standardi. Jedino MISRA pravilo koje nije ispoštovano je 11.2, za koje je re?eno da ga ignorišemo.

Zadatak projekta u crtama:

-Podaci sa dva senzora (levi i desni) se dobijaju automatski na svakih 200ms sa kanala 0 i 1
-Kalibracija se vrši sa kanala 2, koji služi za komunikaciju sa PC-ijem, i preko kog se sistem pali i gasi
-Procentualne udaljenosti se ra?unaju na osnovu o?itavanja senzora i kalibracije
-Na sedmosegmentnim ekranima se prikazuju obe procentualne vrednosti
-Jedna dioda (stubac 1, skroz dole) je signalna, i služi za vizuelnu interpretaciju da li je sistem uklju?en (dioda sija) ili isklju?en (dioda ne sija).
-U zavisnosti od manje procentualne vrednosti, drugi stubac dioda sija razli?itom frekvencijom: ispod 0% - KONTAKT DETEKCIJA (perioda 2*250ms, frekvencija 2Hz), izme?u 0% i 50% - BLISKA DETEKCIJA (perioda 2*500ms, frekvencija 1Hz), izme?u 50% i 100% - UDALJENA DETEKCIJA (perioda 2*1000ms, frekvencija 0,5Hz), i iznad 100% - NEMA DETEKCIJE (diode ne sijaju).
-Zone i procentualne vrednosti sa oba senzora se štampaju na kanalu 2 na svakih 5s

Periferije:
-Pri simulaciji su koriš?eni AdvUniCom za serijsku komunikaciju, Seg7_disp, i LED_bar. 
-Pokretanje LED_bar: za ispravno pokretanje ove periferije za naš kod, u terminalu je potrebno uneti komandu LED_bars_plus.exe rRR, da bi nulti stubac bio tasteri (ulazni), a prvi i drugi diode (izlazni).
-Pokretanje Seg7_disp: u terminalu je potrebno uneti komandu Seg7_Mux 7.
-Pokretanje AdvUniCom: u terminalu je potrebno uneti komande AdvUniCom.exe 0 (ili dvoklikom otvoriti ovaj kanal jer se on automatski pali), AdvUniCom.exe 1, i AdvUniCom.exe 2,da bismo imali sva 3 kanala.

Testiranje sistema:
-Nakon što su sve periferije pokrenute, može se pokrenuti i kod.
-Sistem je inicijalno u upaljenom stanju, može se ugasiti komandom "stopp" u polju za tekst na serijskom kanalu 2 pritiskom na "SEND TEXT", i opet upaliti komandom "start" na istom mestu. 
-Kalibracija se unisi u polju levo od "SEND CODE" na kanalu 2, u formatu \donja_granica\gornja_granica\0d (0d == CR).
-Na kanalima 0 i 1 se levo od teksta "Auto 1" ukuca S1 (kanal 0) i S2 (kanal 1), ?ekira se polje za "Auto 1" na oba kanala, i desno od njega se unosi vrednost u formatu \ef\vrednost_senzora\0d. Potrebno je pritisnuti "ok1" nakon svake promene vrednosti. Na svakih 200ms ?e se vrednosti ponovo o?itavati. 
-Procentualne vrednosti i zone u kojima je distanca je mogu?e pro?itati sa serijskog kanala 2
-Procentualne vrednosti je mogu?e videti i sa sedmosegmentnog displeja (na prva tri displeja ?e se ispisivati prva procentualna vrednost, na slede?em ništa (razdvaja dve vrednosti), i na preostala tri ?e se ispisivati druga procentualna vrednost)
-Zonu senzora koji o?itava manju vrednost je mogu?e pratiti i na LED diodama koje sijaju razli?itom frekvencijom u zavisnosti od zone
-Pri testiranju je najlakše utvrditi da li su podaci obra?eni kako treba tako što ?emo za vrednost senzora uneti neku od granica, te ako je npr senzor1==donja_granica, procentualna vrednost mora biti 0%, i ako je jednaka donjoj onda mora biti 100%. Frekvencije za diode su podešene tako da se što bolje može uo?iti razlika u brzini blinkanja u zavisnosti od zone.

Opis funkcija:

task_ukljuci_iskljuci: funkcija koja vrši "suspend" i "resume" na ostale taskove u projektu, u zavisnosti od toga da li je sistem u upaljenom ili ugašenom stanju. Tako?e, u ovoj funkciji se pali ili gasi signalna dioda koja prati stanje sistema.

task_obrada: funkcija u kojoj se ra?unaju procentualne vrednosti udaljenosti oba senzora u zavisnosti od kalibracije i o?itanih vrednosti sa senzora. Nakon toga se u zavisnosti od tih vrednosti odre?uju zone u kojima se nalaze senzori i vrši dodela perioda za blinkanje dioda. Da bismo vrednost mogli kasnije da ispisujemo na terminal i na displej, u ovoj funkciji se vrši raš?lanivanje vrednosti (na cifre jedinice, desetice i stotine), i njihovo konvertovanje u karaktere koje se pamti u zasebnim nizovima radi daljeg ispisa na terminalu.

Ispis7Seg: funkcija koja ispisuje procentualne vrednosti na sedmosegmentni displej. Za ispis koristi vrednosti dobijene iz obrade ras?lanji9vanjem na jedinice, desetice i stotine. 

RXC_isr_0: funkicija koja o?itava koja vrednost je pristigla sa kanala 0, to jest, senzora 1. U glavnom terminalu se ispisuje šta je o?itano.

RXC_isr_1: sli?no kao prethodna funkcija, samo što o?itava senzor 2 sa kanala 1.

PC_Receive_task: funkcija koja o?itava vrednosti unete za kalibraciju sa kanala 2. Tako?e, o?itava da li je uneta komanda "start" ili "stopp".

Serial0Send_Task: funkcija koja šalje "S1" na kanal 0, radi omogu?avanja automatskog o?itavanja poruke na svakih 200ms.

Serial1Send_Task: sli?no kao prethodna, samo što šalje "S2" na kanal 1.

Serial2Send_Task: funkcija koja na kanal 2 štampa procentualne vrednosti za udaljenost oba senzora (pomo?u nizova karaktera obra?enih u funkciji task_obrada) i kojoj zoni udaljenosti pripadaju (isto na osnovu obra?enih podataka u funkciji za obradu). Poruka treba da se ispisuje na svakih 5s, na ispis poruke se troši 4550ms, pa je dodat delay od 450ms na kraju, da bi vreme do slede?e poruke bilo ta?no 5s.

led_task: ukoliko je vrednost period1 za senzor 1 (dodeljena u funkciji za obradu) manja od period2 za senzor 2 (dodeljena tako?e u funkciji za obradu), onda ?e diode sijati frekvencijom period1*2, to jest sija?e period1 ms, i onda ?e biti ugašena period1 ms. Sli?no se dešava i ako je period2 < period1. U slu?aju da je o?itao da je manja vrednost jednaka 1100, tad ne?e sijati ni jedna dioda, jer je to random vrednost koju smo dodelili za periodu kada se procentualna vrednost nalazi u zoni gde nema detekcije, i tad ?e diode biti u ugašenom stanju dok ne do?e do detekcije u nekoj bližoj zoni.

main_demo: funkcija u kojoj su inicijalizovane sve periferije, definisane interrupt rutine za serijsku komunikaciju, kreirani semafori i svi taskovi, i na kraju pozvan vTaskStartScheduler() koji aktivira planer za raspore?ivanje taskova.
















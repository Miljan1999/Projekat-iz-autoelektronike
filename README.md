## Projekat iz autoelektronike
 Autoelektronika, projekat parking senzor
## Uvod:

 Zadatak projekta je napraviti (isprogramirati) i istestirati RTOS sistem parking senzora. Projekat je kreiran u VisualStudio2019 softveru. Takođe, u projektu su implementirani i MISRA standardi. Jedino MISRA pravilo koje nije ispoštovano je 11.2, za koje je rečeno da ga ignorišemo.

## Zadatak projekta u crtama:

-Podaci sa dva senzora (levi i desni) se dobijaju automatski na svakih 200ms sa kanala 0 i 1
-Kalibracija se vrši sa kanala 2, koji služi za komunikaciju sa PC-ijem, i preko kog se sistem pali i gasi
-Procentualne udaljenosti se računaju na osnovu očitavanja senzora i kalibracije
-Na sedmosegmentnim ekranima se prikazuju obe procentualne vrednosti
-Jedna dioda (stubac 1, skroz dole) je signalna, i služi za vizuelnu interpretaciju da li je sistem uključen (dioda sija) ili isključen (dioda ne sija).
-U zavisnosti od manje procentualne vrednosti, drugi stubac dioda sija različitom frekvencijom: ispod 0% - KONTAKT DETEKCIJA (perioda 2*250ms, frekvencija 2Hz), između 0% i 50% - BLISKA DETEKCIJA (perioda 2*500ms, frekvencija 1Hz), između 50% i 100% - UDALJENA DETEKCIJA (perioda 2*1000ms, frekvencija 0,5Hz), i iznad 100% - NEMA DETEKCIJE (diode ne sijaju).
-Zone i procentualne vrednosti sa oba senzora se štampaju na kanalu 2 na svakih 5s

## Periferije:
-Pri simulaciji su korišćeni AdvUniCom za serijsku komunikaciju, Seg7_disp, i LED_bar. 
-Pokretanje LED_bar: za ispravno pokretanje ove periferije za naš kod, u terminalu je potrebno uneti komandu LED_bars_plus.exe rRR, da bi nulti stubac bio tasteri (ulazni), a prvi i drugi diode (izlazni).
-Pokretanje Seg7_disp: u terminalu je potrebno uneti komandu Seg7_Mux 7.
-Pokretanje AdvUniCom: u terminalu je potrebno uneti komande AdvUniCom.exe 0 (ili dvoklikom otvoriti ovaj kanal jer se on automatski pali), AdvUniCom.exe 1, i AdvUniCom.exe 2,da bismo imali sva 3 kanala.

## Testiranje sistema:
-Nakon što su sve periferije pokrenute, može se pokrenuti i kod.
-Sistem je inicijalno u upaljenom stanju, može se ugasiti komandom "stopp" u polju za tekst na serijskom kanalu 2 pritiskom na "SEND TEXT", i opet upaliti komandom "start" na istom mestu. 
-Kalibracija se unisi u polju levo od "SEND CODE" na kanalu 2, u formatu \donja_granica\gornja_granica\0d (0d == CR).
-Na kanalima 0 i 1 se levo od teksta "Auto 1" ukuca S1 (kanal 0) i S2 (kanal 1), čekira se polje za "Auto 1" na oba kanala, i desno od njega se unosi vrednost u formatu \ef\vrednost_senzora\0d. Potrebno je pritisnuti "ok1" nakon svake promene vrednosti. Na svakih 200ms će se vrednosti ponovo očitavati. 
-Procentualne vrednosti i zone u kojima je distanca je moguće pročitati sa serijskog kanala 2
-Procentualne vrednosti je moguće videti i sa sedmosegmentnog displeja (na prva tri displeja će se ispisivati prva procentualna vrednost, na sledećem ništa (razdvaja dve vrednosti), i na preostala tri će se ispisivati druga procentualna vrednost)
-Zonu senzora koji očitava manju vrednost je moguće pratiti i na LED diodama koje sijaju različitom frekvencijom u zavisnosti od zone
-Pri testiranju je najlakše utvrditi da li su podaci obrađeni kako treba tako što ćemo za vrednost senzora uneti neku od granica, te ako je npr senzor1==donja_granica, procentualna vrednost mora biti 0%, i ako je jednaka donjoj onda mora biti 100%. Frekvencije za diode su podešene tako da se što bolje može uočiti razlika u brzini blinkanja u zavisnosti od zone.

## Opis funkcija:

### task_ukljuci_iskljuci: 
funkcija koja vrši "suspend" i "resume" na ostale taskove u projektu, u zavisnosti od toga da li je sistem u upaljenom ili ugašenom stanju. Takođe, u ovoj funkciji se pali ili gasi signalna dioda koja prati stanje sistema.

### task_obrada: 
funkcija u kojoj se računaju procentualne vrednosti udaljenosti oba senzora u zavisnosti od kalibracije i očitanih vrednosti sa senzora. Nakon toga se u zavisnosti od tih vrednosti određuju zone u kojima se nalaze senzori i vrši dodela perioda za blinkanje dioda. Da bismo vrednost mogli kasnije da ispisujemo na terminal i na displej, u ovoj funkciji se vrši raščlanivanje vrednosti (na cifre jedinice, desetice i stotine), i njihovo konvertovanje u karaktere koje se pamti u zasebnim nizovima radi daljeg ispisa na terminalu.

### Ispis7Seg: 
funkcija koja ispisuje procentualne vrednosti na sedmosegmentni displej. Za ispis koristi vrednosti dobijene iz obrade rasčlanji9vanjem na jedinice, desetice i stotine. 

### RXC_isr_0: 
funkicija koja očitava koja vrednost je pristigla sa kanala 0, to jest, senzora 1. U glavnom terminalu se ispisuje šta je očitano.

### RXC_isr_1: 
slično kao prethodna funkcija, samo što očitava senzor 2 sa kanala 1.

### PC_Receive_task: 
funkcija koja očitava vrednosti unete za kalibraciju sa kanala 2. Takođe, očitava da li je uneta komanda "start" ili "stopp".

### Serial0Send_Task: 
funkcija koja šalje "S1" na kanal 0, radi omogućavanja automatskog očitavanja poruke na svakih 200ms.

### Serial1Send_Task: 
slično kao prethodna, samo što šalje "S2" na kanal 1.

### Serial2Send_Task: 
funkcija koja na kanal 2 štampa procentualne vrednosti za udaljenost oba senzora (pomoću nizova karaktera obrađenih u funkciji task_obrada) i kojoj zoni udaljenosti pripadaju (isto na osnovu obrađenih podataka u funkciji za obradu). Poruka treba da se ispisuje na svakih 5s, na ispis poruke se troši 4550ms, pa je dodat delay od 450ms na kraju, da bi vreme do sledeće poruke bilo tačno 5s.

### led_task: 
ukoliko je vrednost period1 za senzor 1 (dodeljena u funkciji za obradu) manja od period2 za senzor 2 (dodeljena takođe u funkciji za obradu), onda će diode sijati frekvencijom period1*2, to jest sijaće period1 ms, i onda će biti ugašena period1 ms. Slično se dešava i ako je period2 < period1. U slučaju da je očitao da je manja vrednost jednaka 1100, tad neće sijati ni jedna dioda, jer je to random vrednost koju smo dodelili za periodu kada se procentualna vrednost nalazi u zoni gde nema detekcije, i tad će diode biti u ugašenom stanju dok ne dođe do detekcije u nekoj bližoj zoni.

### main_demo: 
funkcija u kojoj su inicijalizovane sve periferije, definisane interrupt rutine za serijsku komunikaciju, kreirani semafori i svi taskovi, i na kraju pozvan vTaskStartScheduler() koji aktivira planer za raspoređivanje taskova.

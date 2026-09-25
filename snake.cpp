#include <curses.h>
#include <stdlib.h>
#include <clocale>

# define Y (int)((float)(wiersze)*0.5)
# define X (int)((float)(kolumny)*0.1)
# define ILOSC_WYNIKOW 5
# define MIN_SZER 32
# define MIN_WYS 12

int kolumny,wiersze;

struct konf
{
 int szybkosc,kolor_weza,kolor_elem;
 bool przenikanie;
 int wysokosc,szerokosc;
 char symb_elem;
};

struct snake
{
 int x,y;
 snake * head, * tail;
};

struct noty
{
 char osoba[10];
 int rezultat;
};

//----------------------------------------------------------------------
// Pobieranie klawiszy (D-Pad / ESC)
int pobierz_klawisz(WINDOW* okno, bool czy_dzwiek);

//----------------------------------------------------------------------
// menu
void wielkosc_okna (WINDOW ** okno);
void logo_snake (WINDOW ** okno, konf ** ustawienia);
int zmien_napis (int klawisz, int wybor ,int max);
void wczytaj_ustawienia(konf ** ustawienia);
void menu (konf ** ustawienia);
int wczytaj_informacje (char * dane);
void informacje (konf ** ustawienia);
void wyniki (konf ** ustawienia);

//----------------------------------------------------------------------
// menu_ustawienia
void menu_ustawienia (konf ** ustawienia);
void zmien_szybkosc(konf ** ustawienia);
void zmien_kolor_weza(konf ** ustawienia);
void zmien_kolor_elem(konf ** ustawienia);
void zmien_okno_gry (konf **ustawienia);
void zmien_przenikanie(konf **ustawienia);
void zapis_ustawien (konf **ustawienia);

//----------------------------------------------------------------------
// gra
void gra (konf ** ustawienia);
void generuj_weza(snake **waz);
void rysuj_weza(WINDOW * okno_gra, snake **waz, konf ** ustawienia);
int sprawdz_weza(snake ** waz);
int przesun_weza(snake ** waz ,snake ** los ,char kierunek, konf ** ustawienia);
int koniec_gry(WINDOW * okno_gra, snake ** waz, konf ** ustawienia);
void generuj_los (snake ** los, konf ** ustawienia);
void rysuj_los (WINDOW * okno_gra, snake ** los, konf ** ustawienia);
void rysuj_pasek_gorny(WINDOW * okno_pasek, snake ** waz, konf ** ustawienia);
void sprawdz_punkt (snake **waz, snake ** los, konf ** ustawienia);
int sprawdz_los (snake ** waz, snake ** los, konf ** ustawienia);

//----------------------------------------------------------------------
// punkty
void generuj_wyniki(void);
void XOR (char * tekst);
void wczytaj_wyniki (noty * dane);
void sprawdz_punkty (int pkt, konf ** ustawienia);
void zmien_rekord (int pkt,int nr,noty * dane, konf ** ustawienia);

//----------------------------------------------------------------------

int pobierz_klawisz(WINDOW* okno, bool czy_dzwiek = false)
{
 int c = (okno != NULL) ? wgetch(okno) : getch();
 if (c == ERR)
 {
  return ERR;
 }
 // Zamiana przycisku "A" (kod 127 lub KEY_BACKSPACE) na Enter ('\n')
 if (c == 127 || c == KEY_BACKSPACE || c == 8)
 {
  return '\n';
 }
 // Obsługa ESC / D-Pada
 if (c == 27)
 {
  wtimeout(okno, 50);
  int c2 = wgetch(okno);
  if (c2 == '[' || c2 == 91)
  {
   int c3 = wgetch(okno);
   if (czy_dzwiek)
   {
    system("aplay -q navigation.wav > /dev/null 2>&1 &");
   }
   switch (c3)
   {
    case 'A': return KEY_UP;
    case 'B': return KEY_DOWN;
    case 'C': return KEY_RIGHT;
    case 'D': return KEY_LEFT;
   }
  }
  return 27;
 }
 // Dźwięk również dla tradycyjnych strzałek z klawiatury
 if (czy_dzwiek && (c == KEY_UP || c == KEY_DOWN || c == KEY_LEFT || c == KEY_RIGHT))
 {
  system("aplay -q navigation.wav > /dev/null 2>&1 &");
 }
 return c;
}

//----------------------------------------------------------------------

int main()
{
 setlocale(LC_ALL, "");
 initscr(); //inicjalizacja ekranu
 curs_set(0); // ukryj kursor
 start_color();   //włączenie trybu kolorowego
 noecho(); //wyłączenie echa na ekran
 keypad(stdscr,TRUE); //support do klawiszy funkcyjnych
 getmaxyx(stdscr,wiersze,kolumny);
 konf * ustawienia;
 ustawienia = new konf;
 wczytaj_ustawienia(&ustawienia);

 if(kolumny < MIN_SZER || wiersze < MIN_WYS)
 {
  endwin();
  system("echo Za małe okno!");
  delete ustawienia;
  return 0;
 }

 menu (&ustawienia);
 endwin(); //zakonczenie pracy w trybie ncurses
 delete ustawienia;
 return 0;
}

//######################################################################
// menu

void wielkosc_okna (WINDOW ** okno)
{
// funkcja sprawdza czy okno nie jest za małe
 getmaxyx( (*okno) ,wiersze,kolumny);
 if(kolumny < MIN_SZER || wiersze < MIN_WYS)
 {
  WINDOW * okno_wielkosc;
  okno_wielkosc=newwin(0, 0, 0, 0);
  wattron(okno_wielkosc,A_BOLD);
  mvwprintw(okno_wielkosc,0,0,"Za małe okno!");
  mvwprintw(okno_wielkosc,2,0,"Minimalne okno %d*%d",MIN_SZER,MIN_WYS);
  do
  {
   wrefresh (okno_wielkosc);
   getmaxyx(okno_wielkosc,wiersze,kolumny);
  }
  while(kolumny < MIN_SZER || wiersze < MIN_WYS);
  delwin(okno_wielkosc);
 }
 else
 {
  werase( (*okno) );
 }
 wrefresh ( (*okno) );
 wmove( (*okno),wiersze-1,0);
}

//----------------------------------------------------------------------

void logo_snake (WINDOW ** okno, konf ** ustawienia)
{
// funkcja wyświetla logo w oknie
 int y;
 char snake1[]="+-------+";
 char snake2[]="| SNAKE |";
 init_pair(1,COLOR_YELLOW,COLOR_BLACK);
 init_pair(3,COLOR_WHITE,COLOR_BLACK);
 wattrset((*okno), COLOR_PAIR(1));
 wattron((*okno),A_BOLD);
 y=(wiersze/5)-1;
 mvwprintw((*okno),y,(kolumny-9)/2,"%s",snake1);
 mvwprintw((*okno),++y,(kolumny-9)/2,"%s",snake2);
 mvwprintw((*okno),++y,(kolumny-9)/2,"%s",snake1);
 wattrset((*okno), COLOR_PAIR(3));
 wattron((*okno),A_BOLD);
}

//----------------------------------------------------------------------

int zmien_napis (int klawisz, int wybor ,int max)
{
// funkcja zmieniająca wartość zmiennej wybor w menu() i menu_ustawienia()
// w zależności od wciśniętego klawisza kursora
 switch (klawisz)
  {
   case KEY_LEFT :
    wybor--;
    break;
   case KEY_RIGHT :
    wybor++;
    break;
  }
  if(wybor<0)
   wybor+=max;
  if(wybor>(max-1))
   wybor%=max;
 return wybor;
}

//----------------------------------------------------------------------

void wczytaj_ustawienia(konf ** ustawienia)
{
// funkcja wczytuje ustawienia gry z pliku
 FILE * plik;
 char temp[100];
 char liczba[6][10]; // Tablica na 6 linii, po max 10 znaków
 int i, j, k;

 if ((plik = fopen("ustawienia", "r")) == NULL)
 {
  // Jeśli brak pliku - tworzymy plik z domyślnymi 6 liniami
  plik = fopen("ustawienia", "w");
  if (plik != NULL) {
   fprintf(plik, "6\n2\n3\n1\n%d\n%d\n", MIN_WYS, MIN_SZER);
   fclose(plik);
  }
  // Domyślne wartości bezpośrednio do struktury:
  (*ustawienia)->szybkosc   = 6;
  (*ustawienia)->kolor_weza = 2;
  (*ustawienia)->kolor_elem = 3;
  (*ustawienia)->przenikanie = true;
  (*ustawienia)->wysokosc   = MIN_WYS;
  (*ustawienia)->szerokosc  = MIN_SZER;
  (*ustawienia)->symb_elem  = '*';
  return;
 }

 // Wczytujemy zawartość pliku
 size_t bytesRead = fread(temp, 1, sizeof(temp) - 1, plik);
 temp[bytesRead] = '\0';
 fclose(plik);

 // Dzielimy bufor `temp` na 6 linii w tablicy `liczba`
 for (i = 0, j = 0; i < 6; i++)
 {
  for (k = 0; temp[j] != '\n' && temp[j] != '\0' && k < 9; k++, j++)
  {
   liczba[i][k] = temp[j];
  }
  liczba[i][k] = '\0';
  if (temp[j] == '\n') j++;
 }

 // Przypisanie do konfiguracji
 (*ustawienia)->szybkosc    = (atoi(liczba[0]) % 11);
 (*ustawienia)->kolor_weza  = (atoi(liczba[1]) % 8);
 (*ustawienia)->kolor_elem  = (atoi(liczba[2]) % 8);
 (*ustawienia)->przenikanie = (atoi(liczba[3]) != 0); // 1 -> true, 0 -> false

 int wys = atoi(liczba[4]);
 int szer = atoi(liczba[5]);
 (*ustawienia)->wysokosc   = (wys < MIN_WYS) ? MIN_WYS : wys;
 (*ustawienia)->szerokosc  = (szer < MIN_SZER) ? MIN_SZER : szer;
 (*ustawienia)->symb_elem  = '*';
}

//----------------------------------------------------------------------

void menu (konf ** ustawienia)
{
 int wybor,klawisz;
 WINDOW * okno_menu;
 noecho();
 okno_menu=newwin(0, 0, 0, 0);
 keypad(okno_menu,TRUE); //support do klawiszy funkcyjnych
 wybor=0;
 do
 {
  wielkosc_okna(&okno_menu);
  logo_snake(&okno_menu,ustawienia);
  mvwprintw(okno_menu,Y-2,X,"  Menu :");
  switch (wybor)
  {
   case 0:
    mvwprintw(okno_menu,Y,X,".: Gra :.");
    break;
   case 1:
    mvwprintw(okno_menu,Y,X,".: Ustawienia :.");
    break;
   case 2:
    mvwprintw(okno_menu,Y,X,".: Wyniki :.");
    break;
   case 3:
    mvwprintw(okno_menu,Y,X,".: Informacje :.");
    break;
   case 4:
    mvwprintw(okno_menu,Y,X,".: Wyjście :.");
    break;
  }
  
  do
   klawisz=pobierz_klawisz(okno_menu, true);
  while(klawisz != KEY_RIGHT && klawisz != KEY_LEFT && klawisz != '\n');

  if(klawisz == '\n')
  {
   switch (wybor)
   {
     case 0:
      werase(okno_menu);
      wrefresh(okno_menu);
      gra (ustawienia);
      menu (ustawienia);
      break;
     case 1:
      menu_ustawienia (ustawienia);
      menu (ustawienia);
      break;
     case 2:
      wyniki (ustawienia);
      menu (ustawienia);
      break;
     case 3:
      informacje (ustawienia);
      menu (ustawienia);
      break;
     case 4:
      break;
   }
  }
  else
   wybor=zmien_napis(klawisz,wybor,5);
 }
 while(klawisz != '\n');
 delwin(okno_menu);
}

//----------------------------------------------------------------------

void wyniki (konf ** ustawienia)
{
// funkcja wyświetla wyniki
int i;
 char znak;
 noty dane[ILOSC_WYNIKOW]; // Tablica na 5 wyników
 WINDOW * okno_wyniki;
 noecho();
 okno_wyniki=newwin(0, 0, 0, 0);
 logo_snake(&okno_wyniki,ustawienia);
 mvwprintw(okno_wyniki,Y-2,X+2,"Wyniki :");
 wczytaj_wyniki( &(dane[0]) );
 // Wyświetlenie 5 wyników
 for(i = 0; i < ILOSC_WYNIKOW; i++)
 {
  mvwprintw(okno_wyniki, Y+i, X+2, "%2d. %s", i+1, dane[i].osoba);
  mvwprintw(okno_wyniki, Y+i, X+18, "%4d", dane[i].rezultat); // Stała pozycja dla punktów
 }
 wrefresh(okno_wyniki);
 do
  znak=pobierz_klawisz(okno_wyniki, true);
 while(znak != '\n' && znak != ' ' && znak != 32);
 delwin(okno_wyniki);
}

//----------------------------------------------------------------------

int wczytaj_informacje (char * dane)
{
// funkcja wczytuje informacje z pliku do stringu dane
 int max,i;
 FILE * plik;
 if ( (plik=fopen("informacje","r")) == NULL)
  return 0;
 else
 {
  fseek(plik,SEEK_SET,0);
  fread(dane,600,1,plik);
  fclose(plik);
  for(i=0;dane[i]!='\n';i++);
  dane[i]='\0';
  // edycja tekstu (niektóre spacje zamieniane na enter)
  // tekst wypełnia okno i nie dzieli wyrazów
  for(i=kolumny-1;i<max;i+=(kolumny-1))
  {
   for(;dane[i]!=' ';i--);
   dane[i]='\n';
  }
 }
 return 1;
}

//----------------------------------------------------------------------

void informacje (konf ** ustawienia)
{
// funkcja wyswietla informacje
 char dane[500],znak;
 WINDOW * okno_informacje;
 noecho();
 okno_informacje=newwin(0, 0, 0, 0);
 wielkosc_okna(&okno_informacje);
 logo_snake(&okno_informacje,ustawienia);
 mvwprintw(okno_informacje,Y-2,X+2,"Informacje :");
 if ( wczytaj_informacje(dane) == 0 )
 {
  mvwprintw(okno_informacje,Y,X+2,"Brak pliku \"informacje\" !");
  wrefresh(okno_informacje);
 }
 else
 {
  mvwprintw(okno_informacje,Y,0,"%s",dane);
 }
 do
  znak=pobierz_klawisz(okno_informacje, true);
 while(znak != '\n' && znak != ' ' && znak != 32);
 wielkosc_okna(&okno_informacje); 
 logo_snake(&okno_informacje,ustawienia);
 mvwprintw(okno_informacje,Y-2,X+2,"Informacje :");
 mvwprintw(okno_informacje,Y,X,"SNAKE 1.0 :");
 mvwprintw(okno_informacje,Y+2,X,"Aurox 9.2 Water");
 mvwprintw(okno_informacje,Y+3,X,"Linux 2.4.20-20.9");
 do
  znak=pobierz_klawisz(okno_informacje, true);
 while(znak != '\n' && znak != ' ' && znak != 32);
 wielkosc_okna(&okno_informacje);
 logo_snake(&okno_informacje,ustawienia);
 mvwprintw(okno_informacje,Y-2,X+2,"Informacje :");
 mvwprintw(okno_informacje,Y,X,"Autor :");
 mvwprintw(okno_informacje,Y+2,X,"student UZ - Paweł Akonom");
 mvwprintw(okno_informacje,Y+3,X,"akus82@o2.pl");
 wrefresh(okno_informacje);
 do
  znak=pobierz_klawisz(okno_informacje, true);
 while(znak != '\n' && znak != ' ' && znak != 32);
 delwin(okno_informacje);
}

//######################################################################
// menu_ustawienia

void menu_ustawienia (konf ** ustawienia)
{
 int wybor,klawisz;   
 WINDOW * okno_menu_ustawienia;
 noecho();
 okno_menu_ustawienia=newwin(0, 0, 0, 0);
 keypad(okno_menu_ustawienia,TRUE);
 init_pair(1,(*ustawienia)->kolor_weza,COLOR_BLACK);
 wybor=0;
 do
 { 
  wielkosc_okna(&okno_menu_ustawienia);
  logo_snake(&okno_menu_ustawienia,ustawienia);
  mvwprintw(okno_menu_ustawienia,Y-2,X+2,"Ustawienia :");
  switch (wybor)
  {
   case 0:
    mvwprintw(okno_menu_ustawienia,Y,X,".: Szybkość węża :. ");
    break;
   case 1:
    mvwprintw(okno_menu_ustawienia,Y,X,".: Kolor węża :. ");
    break;
   case 2:
    mvwprintw(okno_menu_ustawienia,Y,X,".: Kolor elementu :. ");
    break;
   case 3:
    mvwprintw(okno_menu_ustawienia,Y,X,".: Wielkość okna :. ");
    break;
   case 4:
    mvwprintw(okno_menu_ustawienia,Y,X,".: Przenikanie :. ");
    break;
   case 5:
    mvwprintw(okno_menu_ustawienia,Y,X,".: Zapis ustawień :. ");
    break;
   case 6:
    mvwprintw(okno_menu_ustawienia,Y,X,".: Wyjście do menu :. ");
    break;
  }
  
  do
   klawisz=pobierz_klawisz(okno_menu_ustawienia, true);
  while(klawisz != KEY_RIGHT && klawisz != KEY_LEFT && klawisz != '\n');

  if(klawisz == '\n')
  {
   switch (wybor)
   {
    case 0:
     zmien_szybkosc(ustawienia);
     menu_ustawienia(ustawienia);
     break;
    case 1:
     zmien_kolor_weza(ustawienia);
     menu_ustawienia(ustawienia);
     break;
    case 2:
     zmien_kolor_elem(ustawienia);
     menu_ustawienia(ustawienia);
     break;
    case 3:
     zmien_okno_gry(ustawienia);
     menu_ustawienia(ustawienia);
     break;
    case 4:
     zmien_przenikanie(ustawienia);
     menu_ustawienia(ustawienia);
     break;
    case 5:
     zapis_ustawien(ustawienia);
     menu_ustawienia(ustawienia);
     break;
    case 6:
     break;
   }
  }
  else
   wybor=zmien_napis(klawisz,wybor,7);
 }
 while(klawisz != '\n');
 delwin(okno_menu_ustawienia);
}

//----------------------------------------------------------------------

void zmien_szybkosc(konf ** ustawienia)
{
 int klawisz;
 WINDOW * okno_szybkosc;
 noecho();
 okno_szybkosc=newwin(0, 0, 0, 0);
 keypad(okno_szybkosc,TRUE); //support do klawiszy funkcyjnych
 do
 {
  wielkosc_okna(&okno_szybkosc);
  logo_snake (&okno_szybkosc,ustawienia);
  mvwprintw(okno_szybkosc,Y-2,X+2,"Ustawienia :");
  mvwprintw(okno_szybkosc,Y,X,"** Ustaw prędkość **");
  mvwprintw(okno_szybkosc,Y+2,X+10,"%2d",(*ustawienia)->szybkosc);
  wrefresh(okno_szybkosc);
  klawisz=pobierz_klawisz(okno_szybkosc, true);
  switch (klawisz)
  {
   case KEY_LEFT:
    if( (*ustawienia)->szybkosc > 1 )
     (*ustawienia)->szybkosc-=1;
    break;
   case KEY_RIGHT:
    if( (*ustawienia)->szybkosc < 10 )
     (*ustawienia)->szybkosc+=1;
    break;
  }
 }
 while (klawisz!='\n');
 delwin(okno_szybkosc);
} 

//----------------------------------------------------------------------

void zmien_kolor_weza(konf ** ustawienia)
{
 int klawisz;
 char symb = 'o'; // <--- Ustawienie symbolu do podglądu
 WINDOW * okno_kolor_weza;
 noecho();
 okno_kolor_weza=newwin(0, 0, 0, 0);
 keypad(okno_kolor_weza,TRUE); //support do klawiszy funkcyjnych
 do
 {
  wielkosc_okna(&okno_kolor_weza);
  logo_snake (&okno_kolor_weza,ustawienia);
  mvwprintw(okno_kolor_weza,Y-2,X+2,"Ustawienia :");
  mvwprintw(okno_kolor_weza,Y,X,"** Ustaw kolor węża **");
  init_pair(2,(*ustawienia)->kolor_weza,COLOR_BLACK);
  wattrset(okno_kolor_weza, COLOR_PAIR(2));
  wattron(okno_kolor_weza,A_BOLD);
  mvwprintw(okno_kolor_weza,Y+2,X+9,"%c%c%c%c%c",symb,symb,symb,symb,symb);
  wrefresh(okno_kolor_weza);
  klawisz=pobierz_klawisz(okno_kolor_weza, true);
  switch (klawisz)
  {
   case KEY_LEFT:
    if( (*ustawienia)->kolor_weza > 1 )
     (*ustawienia)->kolor_weza-=1;
    break;
   case KEY_RIGHT:
    if( (*ustawienia)->kolor_weza < 7 )
     (*ustawienia)->kolor_weza+=1;
    break;
  }
 }
 while (klawisz!='\n');
 delwin(okno_kolor_weza);
}

//----------------------------------------------------------------------

void zmien_kolor_elem(konf ** ustawienia)
{
 int klawisz;
 WINDOW * okno_kolor_elem;
 noecho();
 okno_kolor_elem=newwin(0, 0, 0, 0);
 keypad(okno_kolor_elem,TRUE); //support do klawiszy funkcyjnych
 do
 {
  wielkosc_okna(&okno_kolor_elem);
  logo_snake (&okno_kolor_elem,ustawienia);
  mvwprintw(okno_kolor_elem,Y-2,X+2,"Ustawienia :");
  mvwprintw(okno_kolor_elem,Y,X,"** Ustaw kolor elementu **");
  init_pair(2,(*ustawienia)->kolor_elem,COLOR_BLACK);
  wattrset(okno_kolor_elem, COLOR_PAIR(2));
  wattron(okno_kolor_elem,A_BOLD);
  mvwprintw(okno_kolor_elem,Y+2,X+13,"%c",(*ustawienia)->symb_elem);
  wrefresh(okno_kolor_elem);
  klawisz=pobierz_klawisz(okno_kolor_elem, true);
  switch (klawisz)
  {
   case KEY_LEFT:
    if( (*ustawienia)->kolor_elem > 1 )
     (*ustawienia)->kolor_elem-=1;
    break;
   case KEY_RIGHT:
    if( (*ustawienia)->kolor_elem < 7 )
     (*ustawienia)->kolor_elem+=1;
    break;
  }
 }
 while (klawisz!='\n');
 delwin(okno_kolor_elem);
}

//----------------------------------------------------------------------

void zmien_okno_gry (konf **ustawienia)
{
 int klawisz;
 WINDOW * okno;
 noecho();
 okno=newwin(0, 0, 0, 0);
 keypad(okno,TRUE);
 init_pair(2,COLOR_BLACK,COLOR_BLACK);
 do
 {
  wielkosc_okna(&okno);
  logo_snake (&okno,ustawienia);
  mvwprintw(okno,Y-2,X+2,"Ustawienia :");
  mvwprintw(okno,Y,X,"** Ustaw wielkość okna **");
  mvwprintw(okno,Y+2,X+9,"%3d * %2d",(*ustawienia)->szerokosc,(*ustawienia)->wysokosc);
  wmove(okno,wiersze-1,0);
  wrefresh(okno);
  klawisz=pobierz_klawisz(okno, true);
  switch (klawisz)
  {
   case KEY_LEFT:
    if( (*ustawienia)->szerokosc > MIN_SZER)
     (*ustawienia)->szerokosc-=1;
    break;
   case KEY_RIGHT:
    if( (*ustawienia)->szerokosc < kolumny)
     (*ustawienia)->szerokosc+=1;
    break;
   case KEY_DOWN:
    if( (*ustawienia)->wysokosc > MIN_WYS)
     (*ustawienia)->wysokosc-=1;
    break;
   case KEY_UP:
    if( (*ustawienia)->wysokosc < wiersze)
     (*ustawienia)->wysokosc+=1;
    break;
  }
 }
 while (klawisz!='\n');
 delwin(okno);
}

//----------------------------------------------------------------------

void zmien_przenikanie(konf ** ustawienia)
{
 int klawisz;
 WINDOW * okno_przenikanie;
 noecho();
 okno_przenikanie=newwin(0, 0, 0, 0);
 keypad(okno_przenikanie,TRUE);
 do
 {
  wielkosc_okna(&okno_przenikanie);
  logo_snake (&okno_przenikanie,ustawienia);
  mvwprintw(okno_przenikanie,Y-2,X+2,"Ustawienia :");
  mvwprintw(okno_przenikanie,Y,X,"** Ustaw przez ściany **");
  
  if ((*ustawienia)->przenikanie)
   mvwprintw(okno_przenikanie,Y+2,X+10,"tak");
  else
   mvwprintw(okno_przenikanie,Y+2,X+10,"nie");
   
  wrefresh(okno_przenikanie);
  klawisz=pobierz_klawisz(okno_przenikanie, true);
  switch (klawisz)
  {
   case KEY_LEFT:
   case KEY_RIGHT:
    (*ustawienia)->przenikanie = !((*ustawienia)->przenikanie);
    break;
  }
 }
 while (klawisz!='\n');
 delwin(okno_przenikanie);
}

//----------------------------------------------------------------------

void zapis_ustawien (konf **ustawienia)
{
 FILE * plik = fopen("ustawienia","w");
 WINDOW * okno_zapis;
 char znak;
 okno_zapis=newwin(0, 0, 0, 0);
 wielkosc_okna(&okno_zapis);
 logo_snake (&okno_zapis,ustawienia);
 mvwprintw(okno_zapis,Y-2,X+2,"Ustawienia :");
 if (plik != NULL)
 {
  fseek(plik,SEEK_SET,0);
  // Linia 1: szybkosc, 2: kolor_weza, 3: kolor_elem, 4: przenikanie (1/0), 5: wysokosc, 6: szerokosc
  fprintf(plik,"%d\n%d\n%d\n%d\n%d\n%d\n", (*ustawienia)->szybkosc, (*ustawienia)->kolor_weza, (*ustawienia)->kolor_elem, (*ustawienia)->przenikanie ? 1 : 0, (*ustawienia)->wysokosc, (*ustawienia)->szerokosc);
  fclose(plik);
  mvwprintw(okno_zapis,Y,X,"** Zapis ustawien **");
  mvwprintw(okno_zapis,Y+2,X,"Ustawienia zapisane !");
 }
 else
 {
  mvwprintw(okno_zapis,Y,X,"** Zapis ustawien **");
  mvwprintw(okno_zapis,Y+2,X,"Blad podczas zapisu !");
 }
 wrefresh(okno_zapis);
 do
  znak=pobierz_klawisz(okno_zapis, true);
 while(znak != '\n' && znak != ' ' && znak != 32);
 delwin(okno_zapis);
}

//######################################################################
// gra

void gra (konf ** ustawienia)
{
 WINDOW * okno_gra;
 WINDOW * okno_pasek;
 int key = ERR, pkt, x, y;
 char kierunek='p';
 snake * waz=NULL, * los=NULL;
 getmaxyx(stdscr,wiersze,kolumny);
 okno_pasek = newwin(1, kolumny, 0, 0);
 
if( (*ustawienia)->wysokosc > wiersze - 1)
  (*ustawienia)->wysokosc = wiersze - 1;
 if( (*ustawienia)->szerokosc > kolumny)
  (*ustawienia)->szerokosc = kolumny;

 // Wyśrodkowanie okna gry w pionie
 if( (*ustawienia)->wysokosc == 0 || (wiersze - (*ustawienia)->wysokosc) < 2)
  y = 1; 
 else
 {
  y = (wiersze - (*ustawienia)->wysokosc) / 2;
  if (y < 1) y = 1; // Górna granica gry nie może zasłaniać wiersza 0
 }
 if( (*ustawienia)->szerokosc == 0 || (kolumny - (*ustawienia)->szerokosc) < 2)
  x = 0;
 else
  x = (kolumny - (*ustawienia)->szerokosc) / 2;
 
 okno_gra=newwin((*ustawienia)->wysokosc,(*ustawienia)->szerokosc, y, x);
 keypad(okno_gra, TRUE);

 // Prędkość gry: im wyższa wartość w ustawieniach, tym wyższa wartość opóźnienia w ms
 int opoznienie = 300 - ((*ustawienia)->szybkosc * 25); 
 if (opoznienie < 30) opoznienie = 30; // Zabezpieczenie przed ujemnym/zbyt małym czasem

 init_pair(8,COLOR_WHITE,COLOR_BLACK);
 generuj_weza(&waz);
 generuj_los(&los, ustawienia);

 do
 { 
  werase(okno_gra);
  rysuj_pasek_gorny(okno_pasek, &waz, ustawienia);
  wattrset(okno_gra, COLOR_PAIR(8));
  box (okno_gra,0,0);
  
  rysuj_weza(okno_gra,&waz,ustawienia);
  rysuj_los(okno_gra,&los,ustawienia);
  wrefresh(okno_gra);

  // Ustawiamy timeout na okno gry
  wtimeout(okno_gra, opoznienie);
  key = pobierz_klawisz(okno_gra, false);

  switch (key)
  {
   case KEY_LEFT : case 'a': case 'A':
    if (kierunek != 'p') kierunek='l';
    break;
   case KEY_RIGHT : case 'd': case 'D':
    if (kierunek != 'l') kierunek='p';
    break;
   case KEY_DOWN : case 's': case 'S':
    if (kierunek != 'g') kierunek='d';
    break;
   case KEY_UP : case 'w': case 'W':
    if (kierunek != 'd') kierunek='g';
    break;
  }

  if( przesun_weza(&waz, &los, kierunek, ustawienia) == 1) 
   break;

 } 
 while(key != ' ' && key != 32); // pętla trwa do wciniecia klawisza X na gamepadzie
 
 pkt=koniec_gry(okno_gra,&waz,ustawienia);
 nodelay(okno_gra, FALSE);
 int k;
 do
 {
  k = pobierz_klawisz(okno_gra, false);
 } 
 while(k != '\n' && k != ' ' && k != 32);
 sprawdz_punkty (pkt,ustawienia);
 
 // zwalnianie pamieci weza
 while(waz->head!=NULL)
  waz=waz->head;
 while(waz->tail!=NULL)
 {
  waz=waz->tail;
  delete waz->head;
 }
 delete waz;
 delete los;
 delwin(okno_gra);
}

//----------------------------------------------------------------------

void generuj_weza(snake **waz)
{
 int i, max;
 max=3; // Domyślna długość początkowa węża
 (*waz)=new snake;
 (*waz)->x=10;
 (*waz)->y=5;
 (*waz)->head=NULL;
 for(i = 1; i <= max; i++)
 {
  (*waz)->tail=new snake;
  (*waz)->tail->head=(*waz);
  (*waz)=(*waz)->tail;
  (*waz)->x=10-i;
  (*waz)->y=5;
 }
 (*waz)->tail=NULL;
}

//----------------------------------------------------------------------

void rysuj_weza(WINDOW * okno_gra, snake **waz, konf ** ustawienia)
{
 init_pair(4,(*ustawienia)->kolor_weza,COLOR_BLACK);
 wattrset(okno_gra,COLOR_PAIR(4)); 	 
 wattron(okno_gra,A_BOLD);
 snake * tmpx = *waz;
 if (tmpx == NULL) return;
 // Przejdź do głowy
 while(tmpx->head != NULL)
  tmpx = tmpx->head;
 // Rysowanie GŁOWY
 mvwprintw(okno_gra, tmpx->y, tmpx->x, "O");
 tmpx = tmpx->tail;
 // Rysowanie TUŁOWIA i OGONA
 while(tmpx != NULL)
 {
  if (tmpx->tail == NULL)
  {
   // OGON
   mvwprintw(okno_gra, tmpx->y, tmpx->x, ".");
  }
  else
  {
   // TUŁÓW
   mvwprintw(okno_gra, tmpx->y, tmpx->x, "o");
  }
  tmpx = tmpx->tail;
 }
}

//----------------------------------------------------------------------

int sprawdz_weza(snake ** waz)
{
 int x,y;
 while((*waz)->head!=NULL)
  (*waz)=(*waz)->head;
 x=(*waz)->x;
 y=(*waz)->y;
 while((*waz)->tail!=NULL)
 {
  if(x==(*waz)->tail->x && y==(*waz)->tail->y)
  {
   return 1;
  }
  (*waz)=(*waz)->tail;
 }
 while((*waz)->head!=NULL)
  (*waz)=(*waz)->head;
 return 0;
}

//----------------------------------------------------------------------

int przesun_weza(snake ** waz ,snake ** los ,char kierunek, konf ** ustawienia)
{
 while((*waz)->head!=NULL)
  (*waz)=(*waz)->head;
 
 (*waz)->head=new snake;
 (*waz)->head->tail=(*waz);
 (*waz)=(*waz)->head;
 (*waz)->head=NULL;

 switch (kierunek)
 {
  case 'l' :
   (*waz)->x=(*waz)->tail->x-1;
   (*waz)->y=(*waz)->tail->y;
   break;
  case 'p' :
   (*waz)->x=(*waz)->tail->x+1;
   (*waz)->y=(*waz)->tail->y;
   break;
  case 'g' :
   (*waz)->x=(*waz)->tail->x;
   (*waz)->y=(*waz)->tail->y-1;
   break;
  case 'd' :
   (*waz)->x=(*waz)->tail->x;
   (*waz)->y=(*waz)->tail->y+1;
   break;
 }

 // Wymiary obszaru roboczego wewnątrz ramki
 int max_x = (*ustawienia)->szerokosc - 2;
 int max_y = (*ustawienia)->wysokosc - 2;

 // OBSŁUGA ŚCIAN I PRZENIKANIA
 if ((*ustawienia)->przenikanie)
 {
  // Przenikanie włączone: teleportacja
  if ((*waz)->x < 1) (*waz)->x = max_x;
  else if ((*waz)->x > max_x) (*waz)->x = 1;

  if ((*waz)->y < 1) (*waz)->y = max_y;
  else if ((*waz)->y > max_y) (*waz)->y = 1;
 }
 else
 {
  // Przenikanie wyłączone: uderzenie w ścianę kończy grę
  if ((*waz)->x < 1 || (*waz)->x > max_x || (*waz)->y < 1 || (*waz)->y > max_y)
  {
   return 1;
  }
 }
 
 if (sprawdz_weza(waz) == 1)
  return 1;

 if( sprawdz_los(waz,los,ustawienia) == 0)
 {
  while((*waz)->tail->tail!=NULL)
   (*waz)=(*waz)->tail;
  delete (*waz)->tail;
  (*waz)->tail=NULL;
 }
 return 0;
}

//----------------------------------------------------------------------

int koniec_gry(WINDOW * okno_gra, snake ** waz, konf ** ustawienia)
{
 int ilosc=0;
 float pkt;
 system("aplay -q game-over.wav > /dev/null 2>&1 &");
 while((*waz)->head!=NULL)
  (*waz)=(*waz)->head;
 while((*waz)->tail!=NULL)
 {
  ilosc++;
  (*waz)=(*waz)->tail;
 }
 ilosc--;
 init_pair(1,COLOR_RED,COLOR_BLACK);
 wattrset(okno_gra,COLOR_PAIR(1));
 wattron(okno_gra,A_BOLD);
 mvwprintw(okno_gra,((*ustawienia)->wysokosc/2)-3,((*ustawienia)->szerokosc-10)/2,"KONIEC GRY");
 mvwprintw(okno_gra,((*ustawienia)->wysokosc/2)-1,((*ustawienia)->szerokosc-11)/2,"długość: %3d",ilosc);

 // Mnożnik punktów: x1.5 jeśli przenikanie jest wyłączone, x1 jeśli włączone
 float mnoznik_scian = ((*ustawienia)->przenikanie) ? 1.0f : 1.5f;
 float pole_powierzchni = (float)((*ustawienia)->wysokosc * (*ustawienia)->szerokosc);
 pkt = (5000.0f * (float)(*ustawienia)->szybkosc / pole_powierzchni) * (float)ilosc * mnoznik_scian;

 mvwprintw(okno_gra,((*ustawienia)->wysokosc/2)+1,((*ustawienia)->szerokosc-11)/2,"punkty: %4d",(int)pkt);
 wrefresh(okno_gra);
 return (int)pkt;
}

//----------------------------------------------------------------------

void generuj_los (snake ** los, konf ** ustawienia)
{
 if (*los == NULL)
 {
     *los = new snake;
 }
 int szer = ((*ustawienia)->szerokosc > 2) ? (*ustawienia)->szerokosc - 2 : 1;
 int wys = ((*ustawienia)->wysokosc > 2) ? (*ustawienia)->wysokosc - 2 : 1;
 (*los)->x = 1 + (rand() % szer);
 (*los)->y = 1 + (rand() % wys);
}

//----------------------------------------------------------------------

void rysuj_los (WINDOW * okno_gra, snake ** los, konf ** ustawienia)
{
// funkcja rysuje element w oknie gry
 init_pair(5,(*ustawienia)->kolor_elem,COLOR_BLACK);
 wattrset(okno_gra,COLOR_PAIR(5));
 wattron(okno_gra,A_BOLD);
 mvwprintw(okno_gra,(*los)->y,(*los)->x,"%c",(*ustawienia)->symb_elem);
}

//----------------------------------------------------------------------

void rysuj_pasek_gorny(WINDOW * okno_pasek, snake ** waz, konf ** ustawienia)
{
 int dlugosc = 0;
 snake * tmp = *waz;
 if (tmp == NULL) return;
 // Policz długość węża
 while (tmp->head != NULL) tmp = tmp->head;
 while (tmp->tail != NULL) {
  dlugosc++;
  tmp = tmp->tail;
 }
 // Oblicz punktację na żywo
 float mnoznik_scian = ((*ustawienia)->przenikanie) ? 1.0f : 1.5f;
 float pole_powierzchni = (float)((*ustawienia)->wysokosc * (*ustawienia)->szerokosc);
 float pkt = (5000.0f * (float)(*ustawienia)->szybkosc / pole_powierzchni) * (float)dlugosc * mnoznik_scian;
 init_pair(9, COLOR_WHITE, COLOR_BLACK);
 wattrset(okno_pasek, COLOR_PAIR(9));
 wattron(okno_pasek, A_BOLD);
 werase(okno_pasek);
 // Wyświetlenie punktów w lewym górnym rogu terminala (y=0, x=0)
 mvwprintw(okno_pasek, 0, 0, " Długość: %-3d   Punkty: %-5d", dlugosc, (int)pkt);
 wrefresh(okno_pasek);
}

//----------------------------------------------------------------------

void sprawdz_punkt (snake **waz, snake ** los, konf ** ustawienia)
{
 // funkcja sprawdza czy element nie został wylosowany na wężu
 bool kolizja;
 do
 {
  kolizja = false;
  // Przejdź do głowy węża
  snake * tmp = *waz;
  while (tmp->head != NULL)
   tmp = tmp->head;
  // Przeszukaj całe ciało węża (od głowy po sam koniec ogona)
  while (tmp != NULL)
  {
   if (tmp->x == (*los)->x && tmp->y == (*los)->y)
   {
    kolizja = true;
    generuj_los(los, ustawienia); // Wygeneruj nowy punkt i sprawdź od nowa
    break;
   }
   tmp = tmp->tail;
  }
 }
 while (kolizja);
}

//----------------------------------------------------------------------

int sprawdz_los (snake ** waz, snake ** los, konf ** ustawienia)
{
// funkcja generuje defoultowe wyniki w razie braku pliku
 while((*waz)->head!=NULL)
  (*waz)=(*waz)->head;
 
 if((*waz)->x == (*los)->x && (*waz)->y == (*los)->y)
 {
  system("aplay -q bite.wav > /dev/null 2>&1 &");
  generuj_los(los, ustawienia);
  sprawdz_punkt(waz, los, ustawienia);
  return 1;
 }
 return 0;
}

//######################################################################
// punkty

void generuj_wyniki(void)
{
// funkcja generuje domyślne wyniki w razie braku pliku
 FILE * plik = fopen("wyniki","w");
 if (plik == NULL) return;
 char temp[] = "Pyton\n50\nBoa\n40\nKobra\n30\nZaskroniec\n20\nŻmija\n10\n";
 XOR (temp);
 fprintf(plik,"%s",temp);
 fclose(plik);
}

//----------------------------------------------------------------------

void XOR (char * tekst) // Ciiii ;)
{
 int i;
 for(i=0;tekst[i]!=0;i++)
  if(tekst[i]!='\n')
   tekst[i]^=0x53;
}

//----------------------------------------------------------------------

void wczytaj_wyniki (noty * dane)
{
 int i,j,k;
 FILE * plik;
 char temp[50],liczba[5];
 // funkcja wczytuje dane o wynikach do stróktury
 if ( (plik=fopen("wyniki","r")) == NULL)
 {
  generuj_wyniki();
  wczytaj_wyniki (dane);
 }
 else
 {
  fseek(plik,SEEK_SET,0);
  size_t bytes = fread(temp, 1, sizeof(temp) - 1, plik);
  temp[bytes] = '\0';
  XOR(temp);
  fclose(plik); 
  // 10 linii tekstu (5 nazw + 5 wartości)
  for(i=0, k=0; k!=10 && temp[i]!='\0'; i++)
  {
   if( temp[i]=='\n')
    k++;
  }
  temp[i]=0;
  // wczytywanie 5 rekordów z bufora do struktury
  for(i=0, k=0; k < ILOSC_WYNIKOW; k++)
  {
   for(j=0; temp[i]!='\n' && temp[i]!='\0'; i++, j++)
    dane[k].osoba[j]=temp[i];
   dane[k].osoba[j]='\0';
   if(temp[i]=='\n') i++;
   for (j=0; temp[i]!='\n' && temp[i]!='\0'; j++, i++)
    liczba[j]=temp[i];
   liczba[j]='\0';
   dane[k].rezultat=atoi(liczba);
   if(temp[i]=='\n') i++;
  }
 }
}

//----------------------------------------------------------------------

void sprawdz_punkty (int pkt, konf ** ustawienia)
{
 int i;
 noty dane[ILOSC_WYNIKOW];
 wczytaj_wyniki( &(dane[0]) );
 // sprawdzenie rezultatu z dotychczasowymi wynikami
 for(i = 0; i < ILOSC_WYNIKOW; i++)
 {
  if(pkt > dane[i].rezultat)
  {
   zmien_rekord (pkt, i, &(dane[0]), ustawienia);
   break;
  }
 }
}

//----------------------------------------------------------------------

void zmien_rekord (int pkt,int nr,noty * dane, konf ** ustawienia)
{
 int k;
 char temp[256];
 const char* nazwa_weza;
 WINDOW * okno_wpis;
 FILE * plik=fopen("wyniki","w");
 okno_wpis=newwin(0, 0, 0, 0);
 wielkosc_okna(&okno_wpis);
 logo_snake(&okno_wpis,ustawienia);
// Przypisanie nazwy węża na podstawie zajętego miejsca (nr: 0 = 1. miejsce, 1 = 2. miejsce, 2 = 3. miejsce)
 switch (nr)
 {
  case 0:
   nazwa_weza = "Pyton";
   break;
  case 1:
   nazwa_weza = "Boa";
   break;
  case 2:
  default:
   nazwa_weza = "Kobra";
   break;
  case 3:
   nazwa_weza = "Zaskroniec";
   break;
  case 4:
   nazwa_weza = "Żmija";
   break;
 }
 mvwprintw(okno_wpis,Y,X,"Nowy rekord, %d miejsce:",nr+1);
 mvwprintw(okno_wpis, Y + 2, X, "Twój wąż to %s", nazwa_weza);
 wrefresh(okno_wpis);
 // Czekamy na zatwierdzenie przyciskiem A / Enter / D-Pad
 pobierz_klawisz(okno_wpis, true);
// Przesunięcie niższych rekordów w dół (od indeksu 4 do nr)
 for(k = ILOSC_WYNIKOW - 1; k > nr; k--)
 {
  dane[k].rezultat = dane[k-1].rezultat;
  sprintf(dane[k].osoba, "%s", dane[k-1].osoba);
 }
 // Zapisanie nowej nazwy węża i punktów
 sprintf(dane[nr].osoba, "%s", nazwa_weza);
 dane[nr].rezultat = pkt;
 getmaxyx(stdscr,wiersze,kolumny); // pobranie ilości kolumn i wierszy
 sprintf(temp,"%s\n%d\n%s\n%d\n%s\n%d\n%s\n%d\n%s\n%d\n",dane[0].osoba,dane[0].rezultat,dane[1].osoba,dane[1].rezultat,dane[2].osoba,dane[2].rezultat,dane[3].osoba,dane[3].rezultat,dane[4].osoba,dane[4].rezultat);
 XOR(temp);
 fprintf(plik,"%s",temp);
 delwin(okno_wpis);
 fclose(plik);
}

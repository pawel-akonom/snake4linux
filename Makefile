SCIEZKA_INSTALACJI="$(HOME)/snake"
SCIAZKA_NCURSES="/usr/include"

sprawdz_ncurses :
	@if [ ! -f $(SCIAZKA_NCURSES)/ncurses.h ]; then \
		echo "BŁĄD: Brak pliku $(SCIAZKA_NCURSES)/ncurses.h"; \
		echo "Zainstaluj pakiet libncurses-dev (Debian/Ubuntu) lub ncurses-devel (Fedora/RHEL) w systemie."; \
		exit 1; \
	fi

snake : sprawdz_ncurses snake.cpp
	@echo "Kompilacja gry Snake 1.0"
	g++ snake.cpp -lncursesw -I$(SCIAZKA_NCURSES) -o $@

install : snake
	@echo "Instalacja gry Snake 1.0 w $(SCIEZKA_INSTALACJI)..."
	mkdir -p $(SCIEZKA_INSTALACJI)
	cp ./snake $(SCIEZKA_INSTALACJI)/
	cp ./ustawienia $(SCIEZKA_INSTALACJI)/ 2>/dev/null || true
	cp ./informacje $(SCIEZKA_INSTALACJI)/ 2>/dev/null || true
	cp ./wyniki $(SCIEZKA_INSTALACJI)/ 2>/dev/null || true
	cp ./bite.wav $(SCIEZKA_INSTALACJI)/ 2>/dev/null || true
	cp ./navigation.wav $(SCIEZKA_INSTALACJI)/ 2>/dev/null || true
	cp ./game-over.wav $(SCIEZKA_INSTALACJI)/ 2>/dev/null || true
	@echo "Gra została pomyślnie zainstalowana w $(SCIEZKA_INSTALACJI)"

uninstall :
	$(RM) $(SCIEZKA_INSTALACJI)/snake
	$(RM) $(SCIEZKA_INSTALACJI)/ustawienia
	$(RM) $(SCIEZKA_INSTALACJI)/informacje
	$(RM) $(SCIEZKA_INSTALACJI)/wyniki
	$(RM) $(SCIEZKA_INSTALACJI)/bite.wav
	$(RM) $(SCIEZKA_INSTALACJI)/navigation.wav
	$(RM) $(SCIEZKA_INSTALACJI)/game-over.wav
	rmdir $(SCIEZKA_INSTALACJI)
	@echo "Gra Snake 1.0 odinstalowana"




	

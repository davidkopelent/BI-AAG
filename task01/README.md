# Doplněk jazyka definovaného konečným automatem

Algoritmus determinizace konečného automatu je klíčovým nástrojem v teorii formálních jazyků. Nalézá rozsáhlé praktické využití ve zpracování textu, analýze dat a v různých oblastech informatiky. Tím, že dovoluje převádět nedeterministické automaty na deterministické, umožňuje efektivní analýzu jazyků, což je klíčové v kompilátorech, analýze textu, vyhledávání regulárních výrazů, řízení sítí, databázových systémech a mnoha dalších aplikacích.

V některých situacích se však vyplatí vytvářet automaty pro jazyk tak, že najdeme automat doplněk jazyka a poté algoritmicky vytvoříme automat pro původní jazyk.

Úkolem je implementovat dvě funkce:

- DFA complement ( const MISNFA & nfa );: Funkce vrátí deterministický konečný automat bez nedosažitelných a bez zbytečných stavů, který přijímá doplněk jazyka automatu nfa.
- bool run ( const DFA & dfa, const Word & word );: Funkce vrátí true, pokud automat dfa přijímá slovo word, jinak vrátí false.

Vstupem, resp. výstupem algoritmů jsou automaty v podobě struktur MISNFA, resp. DFA reprezentující nedeterministický konečný automat s více poč. stavy, resp. deterministický konečný automat. Tyto struktury jsou definovány v testovacím prostředí, vizte ukázku níže. Pro zjednodušení jsou stavy definovány jako hodnoty typu State a symboly abecedy jako hodnoty typu Symbol. Dále testovací prostředí definuje typ Word reprezentující slovo. Tento typ je jen alias pro std::vector<Symbol>.

Porovnání automatů s referenčním výsledkem se provádí přes převod na minimální deterministický konečný automat. Vaše výstupy se mohou lišit (např. v pojmenování stavů). Po převedení na minimální automat, které provede testovací prostředí, však musí dát ekvivalentní automat (pojmenování stavů nehraje při porovnání roli).

Je zaručeno, že jako parametry funkce complement budou validní nedeterministické konečné automaty s více počátečními stavy, tedy:

 - množiny stavů (MISNFA::m_States) a počátečních stavů (MISNFA::m_InitialStates) budou neprázdné,
 - počáteční a koncové stavy z množin MISNFA::m_InitialStates a MISNFA::m_FinalStates budou také prvky množiny stavů MISNFA::m_States,
 - pokud nebude pro nějaký stav q a symbol abecedy a definovaný přechod v automatu, pak v mapě MISNFA::m_Transitions nebude ke klíči (q, a) přiřazená hodnota prázdná množina, ale tento klíč nebude vůbec existovat,
 - v mapě přechodů MISNFA::m_Transitions se vyskytují také jen prvky, které jsou specifikovány v množině symbolů abecedy a stavů.
 - Výsledný DFA musí také splňovat podmínky definice automatu, tedy musí platit to samé co výše pro MISNFA (až na zřejmé změny kvůli rozdílným definicím počátečního stavu a přechodové funkce).

Do funkce run budou jako parametry vstupovat validní deterministické automaty a slova nad libovolnými abecedami.

Pokud je jazyk automatu prázdný jazyk, pak odevzdávejte jednostavový automat nad stejnou abecedou, jako je původní automat. Vstupní abecedu automatu není nikdy potřeba měnit.

Odevzdávejte zdrojový soubor, který obsahuje implementaci požadovaných funkcí Do zdrojového souboru přidejte i další Vaše podpůrné funkce, které jsou z implementovaných funkcí volané. Funkce bude volaná z testovacího prostředí, je proto důležité přesně dodržet zadané rozhraní funkce. Za základ pro implementaci použijte kód z ukázky níže. V kódu chybí vyplnit definice implementovaných funkcí a případné další podpůrné funkce. Ukázka obsahuje testovací funkci main, uvedené hodnoty jsou použité při základním testu. Všimněte si, že vkládání hlavičkových souborů, struktur DFA a MISNFA a funkce main jsou zabalené v bloku podmíněného překladu (#ifdef/#endif). Prosím, ponechte bloky podmíněného překladu i v odevzdávaném zdrojovém souboru. Podmíněný překlad Vám zjednoduší práci. Při kompilaci na Vašem počítači můžete program normálně spouštět a testovat. Při kompilaci na Progtestu funkce main a vkládání hlavičkových souborů "zmizí", tedy nebude kolidovat s hlavičkovými soubory a funkcí main testovacího prostředí.

Pro základ implementace můžete využít soubor ke stažení níže v sekci Vzorová data. Tento soubor obsahuje také několik základních testů, mějte však na paměti, že testuje pouze výsledky funkce run. Testy pro funkci complement nejsou dodávány, protože Vaše výstupy se mohou výrazně lišit podle zvolených algoritmů a pojmenování stavů. Testování funkce complement můžete provést pomocí nástroje ALT.

Váš program bude spouštěn v omezeném testovacím prostředí. Je omezen dobou běhu (limit je vidět v logu referenčního řešení) a dále je omezena i velikost dostupné paměti.

## Poznámky:
Potřebné algoritmy pro tento úkol byly probrány v přednáškách a na cvičení.
Mějte na paměti, že vstupy jsou různé validní automaty podle definice z přednášky. Koncový stav může být například jen jeden a to v nedosažitelné části automatu (tedy jazyk automatu může být prázdný) nebo také nemusí být koncový žádný stav.
Výstupem funkce complement musí být validní automat. Dejte pozor zejména na správné inicializace atributů DFA vzhledem k omezením výše.
Časový limit je nastaven poměrně benevolentně. Měl by postačit i pro naivní implementaci algoritmů z přednášky.
Nápovědy, které vypisují automaty, jsou vypsány ve formátu pro nástroj ALT. Můžete tento výstup využít pro debugging v tomto nástroji. Vizte dokumentaci formátu

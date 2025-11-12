#include "DictStrings.hpp"

// Lithuanian translations for TaxonomyDict
const char* const DICT_STRINGS_LT[] = {
    // Window titles
    "Taksonomijos žodyno generatorius",

    // Menu items
    "&Failas",
    "&Išeiti\tCtrl+Q",
    "Uždaryti programą",
    "&Pagalba",
    "&Apie\tF1",
    "Apie šią programą",

    // Button labels
    "Atsisiųsti archyvą",
    "Įkelti esamą archyvą...",
    "Generuoti žodyną",
    "Naršyti...",

    // Dialog titles
    "Apie",
    "Išsaugoti archyvą kaip...",
    "Pasirinkite archyvo failą",
    "Klaida",
    "Neteisingas URL",
    "Gijos klaida",

    // Section titles
    "Archyvo atsisiuntimas",
    "Kalbų pasirinkimas",
    "Išvestis",

    // Control labels
    "iNaturalist taksonomijos URL:",
    "Šaltinio kalba:",
    "Tikslinės kalbos:",
    "Išvesties kelias:",

    // Checkbox labels
    "Įtraukti mokslinius pavadinimus",
    "Filtruoti tik tikslines kalbas",

    // Static values
    "Mokslinis pavadinimas",
    "https://www.inaturalist.org/taxa/inaturalist-taxonomy.dwca.zip",
    "./output",

    // Status messages
    "Paruošta",
    "Atsisiunčiama...",
    "Atsisiuntimo gija paleista...",
    "Atsisiunčiama... %1$d%%",
    "Atsisiuntimas sėkmingai baigtas! Naudokite 'Įkelti esamą archyvą' norėdami jį apdoroti.",
    "Atsisiuntimas nepavyko: %1$s",
    "Įkeliamas archyvas...",
    "Nepavyko paleisti įkėlimo gijos",
    "Nepavyko įkelti archyvo",
    "Pradedama žodyno generacija...",
    "Žodyno generacija nepavyko",

    // Error messages
    "Įveskite atsisiuntimo URL",
    "URL turi prasidėti http:// arba https://",
    "Nepavyko paleisti atsisiuntimo gijos (klaida: %1$d)",
    "Nepavyko paleisti įkėlimo gijos",
    "Pasirinkite šaltinio kalbą",
    "Pasirinkite bent vieną tikslinę kalbą arba įtraukite mokslinius pavadinimus",
    "Nepavyko paleisti žodyno generavimo gijos",
    "Nepavyko įkelti archyvo",
    "Žodyno generacija nepavyko",
    "Nepavyko inicializuoti CURL",

    // About dialog message (note: version will be prepended at runtime)
    "\n\nGeneruoja vertimų žodynus iš iNaturalist taksonomijos duomenų.",

    // File dialog filters
    "ZIP failai (*.zip)|*.zip",

    // Progress messages - Archive operations
    "Atidaromas archyvas: %1$s",
    "Archyve yra %1$d failų",
    "Rastas failas: %1$s",
    "Pradedamas taksonomijos duomenų įkėlimas...",
    "Ieškoma kalbų failų...",
    "Rasta %1$d kalbų failų",
    "Apdorojamas kalbos failas: %1$s",
    "Išgauta kalba: %1$s",
    "Archyvas sėkmingai įkeltas! Rasta %1$d kalbų, %2$d taksonomijos įrašų",
    "Nepavyko atidaryti archyvo",

    // Progress messages - Taxonomy loading
    "Bandoma įkelti taksonomijos duomenis...",
    "Nepavyko perskaityti 'taxa.csv', bandoma 'taxonomy.csv'",
    "Nepavyko perskaityti 'taxonomy.csv', bandoma 'taxa.txt'",
    "Nepavyko perskaityti 'taxa.txt'",
    "Taksonomijos failas sėkmingai perskaitytas, turinio ilgis: %1$d baitų",
    "Analizuojami taksonomijos CSV duomenys...",
    "Analizės rezultatas: %1$s, taksonomijos įrašų: %2$d",

    // Progress messages - Dictionary generation
    "Pradedama žodyno generacija...",
    "Žodyno generacija nepavyko - įrašų nerasta",
    "Žodynas sudarytas su %1$d įrašais, kuriamas išvesties katalogas...",
    "Išvesties katalogas paruoštas, generuojami failai...",
    "Generuojamas TSV failas...",
    "Generuojami StarDict failai...",
    "Žodyno generacija sėkmingai užbaigta! Sugeneruota %1$d įrašų",
    "Įkeliami šaltinio kalbos duomenys: %1$s",
    "Nepavyko įkelti mokslinių pavadinimų duomenų",
    "Nepavyko įkelti šaltinio kalbos duomenų",
    "Įkelta %1$d šaltinio įrašų, įkeliamos tikslinės kalbos...",
    "Praleistas mokslinis pavadinimas kaip tikslinis (naudokite 'Įtraukti mokslinius pavadinimus')",
    "Įkeliama tikslinė kalba: %1$s",
    "Įkelta %1$d įrašų kalbai %2$s",
    "Kuriami žodyno suženklinimai...",
    "Apdorojamas įrašas %1$d iš %2$d...",
    "Žodyno sudarymas baigtas, rasta %1$d įrašų",

    // Search UI
    "Paieška: %1$s",
};

// Compile-time validation
VALIDATE_TRANSLATION_TABLE(DictStringId, DICT_STRINGS_LT);

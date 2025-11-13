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
    "Atsisiunčiama... %d%%",
    "Atsisiuntimas sėkmingai baigtas! Naudokite 'Įkelti esamą archyvą' norėdami jį apdoroti.",
    "Atsisiuntimas nepavyko: %s",
    "Įkeliamas archyvas...",
    "Nepavyko paleisti įkėlimo gijos",
    "Nepavyko įkelti archyvo",
    "Pradedama žodyno generacija...",
    "Žodyno generacija nepavyko",

    // Error messages
    "Įveskite atsisiuntimo URL",
    "URL turi prasidėti http:// arba https://",
    "Nepavyko paleisti atsisiuntimo gijos (klaida: %d)",
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
    "Atidaromas archyvas: %s",
    "Archyve yra %d failų",
    "Rastas failas: %s",
    "Pradedamas taksonomijos duomenų įkėlimas...",
    "Ieškoma kalbų failų...",
    "Rasta %d kalbų failų",
    "Apdorojamas kalbos failas: %s",
    "Išgauta kalba: %s",
    "Archyvas sėkmingai įkeltas! Rasta %d kalbų, %d taksonomijos įrašų",
    "Nepavyko atidaryti archyvo",

    // Progress messages - Taxonomy loading
    "Bandoma įkelti taksonomijos duomenis...",
    "Nepavyko perskaityti 'taxa.csv', bandoma 'taxonomy.csv'",
    "Nepavyko perskaityti 'taxonomy.csv', bandoma 'taxa.txt'",
    "Nepavyko perskaityti 'taxa.txt'",
    "Taksonomijos failas sėkmingai perskaitytas, turinio ilgis: %d baitų",
    "Analizuojami taksonomijos CSV duomenys...",
    "Analizės rezultatas: %s, taksonomijos įrašų: %d",

    // Progress messages - Dictionary generation
    "Pradedama žodyno generacija...",
    "Žodyno generacija nepavyko - įrašų nerasta",
    "Žodynas sudarytas su %d įrašais, kuriamas išvesties katalogas...",
    "Išvesties katalogas paruoštas, generuojami failai...",
    "Generuojamas TSV failas...",
    "Generuojami StarDict failai...",
    "Žodyno generacija sėkmingai užbaigta! Sugeneruota %d įrašų",
    "Įkeliami šaltinio kalbos duomenys: %s",
    "Nepavyko įkelti mokslinių pavadinimų duomenų",
    "Nepavyko įkelti šaltinio kalbos duomenų",
    "Įkelta %d šaltinio įrašų, įkeliamos tikslinės kalbos...",
    "Praleistas mokslinis pavadinimas kaip tikslinis (naudokite 'Įtraukti mokslinius pavadinimus')",
    "Įkeliama tikslinė kalba: %s",
    "Įkelta %d įrašų kalbai %s",
    "Kuriami žodyno suženklinimai...",
    "Apdorojamas įrašas %d iš %d...",
    "Žodyno sudarymas baigtas, rasta %d įrašų",

    // Search UI
    "Paieška: %s",
};

// Compile-time validation
VALIDATE_TRANSLATION_TABLE(DictStringId, DICT_STRINGS_LT);

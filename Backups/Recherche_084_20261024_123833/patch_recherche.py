from pathlib import Path
import sys

path = Path(sys.argv[1])
data = path.read_bytes()

# Conserver le format d'encodage et les fins de ligne du fichier.
if data.startswith(b"\xef\xbb\xbf"):
    encoding = "utf-8-sig"
elif data.startswith(b"\xff\xfe"):
    encoding = "utf-16"
else:
    encoding = "utf-8"

text = data.decode(encoding)
newline = "\r\n" if "\r\n" in text else "\n"

def replace_once(old, new, label):
    global text
    old = old.replace("\n", newline)
    new = new.replace("\n", newline)
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: attendu 1 bloc, trouve {count}")
    text = text.replace(old, new, 1)
    print("OK :", label)

# Ajouter les noms affichés aux termes recherchables.
old_search = """        EntrySearchTexts.Add(
            FString::Printf(
                TEXT("%s %s"),
                *Entry.Category,
                *Entry.SchematicClassPath
            )
        );"""

new_search = """        EntrySearchTexts.Add(
            FString::Printf(
                TEXT("%s %s"),
                *Entry.Category,
                *Entry.SchematicClassPath
            )
        );"""

# Le bloc initial reste identique ici.
# Nous compléterons le texte après création du NameText.
if text.count(old_search.replace("\n", newline)) != 1:
    raise RuntimeError("Bloc de recherche initial introuvable")

old_name = """        StatusText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("PRIX : %d CREDITS CONTRACTORY"),
                    Entry.PriceCredits
                )
            )
        );"""

new_name = """        // CONTRACTORY_SEARCH_LOCALIZED_084
        // Ajouter le nom réellement affiché à la recherche.
        if (EntrySearchTexts.IsValidIndex(Index))
        {
            EntrySearchTexts[Index] += TEXT(" ");
            EntrySearchTexts[Index] += NameText->GetText().ToString();
        }

""" + old_name

replace_once(old_name, new_name, "Recherche par nom localise")

# Ajouter un suivi des titres de catégorie sans changer les structures du header.
old_title = """            CatalogueScroll->AddChild(
                CategoryTitle
            );"""

new_title = """            CatalogueScroll->AddChild(
                CategoryTitle
            );

            // CONTRACTORY_SEARCH_CATEGORY_084
            CategoryTitle->SetVisibility(ESlateVisibility::Visible);"""

replace_once(old_title, new_title, "Repere titres de categories")

# Vérifier que la fonction de recherche existe.
if "void UContractoryBoutiqueWidget::HandleSearchChanged(" not in text:
    raise RuntimeError("Fonction HandleSearchChanged introuvable")

# Ecriture atomique après toutes les validations.
temporary = path.with_suffix(".cpp.tmp")
temporary.write_bytes(text.encode(encoding))
temporary.replace(path)

print("PATCH APPLIQUE :", path)

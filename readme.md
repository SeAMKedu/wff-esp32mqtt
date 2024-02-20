[seamk_logo]:       /img/seamk_logo.svg
[epliitto_logo]:    /img/epliitto_logo.jpg
[eakr_eu_logo]:     /img/eakr_eu_logo.jpg

[![DOI](https://zenodo.org/badge/737607360.svg)](https://zenodo.org/doi/10.5281/zenodo.10682553)

# Analogia signaalit pilveen

Ruokalaboratorion homogenisaattorissa on valmiit liitännät, joista voi lukea 4-20mA signaalin paineantureista. Tässä repositoryssä on kuvattu ESP32 pohjainen lähetin, joka osaa lukea nämä signaalit ja lähettää ne edelleen MQTT viesteinä eteenpäin. 

## Piirilevy

PCB kansiosta löytyy KiCAD piirustukset sovittimesta, johon voi laittaa ESP32 Dev Kitin kiinni sekä kytkeä 24V virran ja kaksi mA-signaalia. HUOM! Tämä on laajempaa jakelua varten piirretty uudelleen KiCADilla, eikä sitä ole testattu. Testattu prototyyppi tehtiin EasyEDAlla mikä on hieman rajattu yhteen piirilevyvalmistajaan. KiCAD versiossa on myös lisätty OneWire liitäntä lämpötila-anturia varten, mikä ei ole käytössä ohjelmassa.

## Ohjelma

Ohjelma ESP32:lle tietojen lähettämistä varten on kirjoitettu PlatformIO ympäristössä. Asenna se ja avaa projekti. `platformio.ini` tiedostossa on muuttujat WIFI-verkon ja MQTT palvelimen asetuksille. Aseta sinne itsellesi sopivat arvot.

## Wise Frami Food hanke

Tämä ohjelma luotiin osana Wise Frami Food hanketta, jossa rakennettiin piloitointiympäristö Frami Food Labin yhteyteen.

![eakr_eu_logo] 

---

![epliitto_logo]

---

![seamk_logo]

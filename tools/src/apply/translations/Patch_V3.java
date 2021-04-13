package apply.translations;

import static apply.translations.TextrumentLocalePatchs.*;


// update from Npp789 -> Npp795
public class Patch_V3 implements ActionApplier {
	Action action;
	@Override
	public void pushActions() {

		//将保存对话框中的文件扩展名过滤器设为 *.*
		action = pushActionByPathFieldedIDForFieldInTag("{zh_CN:'将保存对话框中的文件扩展名设为 *.*'}", MODIFY, "id", 6351, "name", "Item", "Native-Langue", "Dialog", "Preference", "MISC");

		for (int i = 43062; i < 43066; i++) {
			// styles
			action = pushActionByPathFieldedIDForFieldInTag(null, DELETE, "id", i, "name", "Item", "Native-Langue", "Menu", "Main", "Commands");
		}


//打开所在文件夹工作区
		action = pushActionByPathFieldedIDForFieldInTag("{bg:'Отваряне на директорията като работно място', pl:'Otwórz folder zawierający w obszarze roboczym', da:'Åbn indeholdende mappe som arbejdsområde', de:'Enthaltenden Ordner als Arbeitsbereich öffnen', ru:'Открыть папку документа как Проект', fr:'Ouvrir le répertoire du fichier en cours en tant qu\\'espace de travail', ko:'포함된 폴더를 작업 영역으로 열기', nl:'Bevattende map als werkruimte openen', cs:'Otevřít složku obsahující akt. soubor jako Pracovní plochu', co:'Apre u cartulare cuntenendu u schedariu cum’è spaziu di travagliu', ja:'ファイルを含むフォルダーをワークスペースとして開く', sk:'Otvoriť priečinok súboru ako pracovný priestor', sl:'Odpri vsebino mape as Workspace', tr:'İçeren Klasörü Çalışma Alanı Olarak Aç', uk:'Відкрити теку вмісту як робочу область', es:'Open Containing Folder as Workspace', it:'Apri cartella contenuta come Workspace', en:'Open Containing Folder as Workspace', en1:'Open Containing Folder as Workspace', zh_CN:'打开所在文件夹工作区', venetian:'Vèrxi carteƚa contegnù cofà Workspace', hongKongCantonese:'喺資料夾工作區打開', abkhazian:'Иаарттәуп Адокумент акаталог Проект ҳасабла'}", INSERT, "CMID", 23, "name", "Item", "Native-Langue", "Menu", "TabBar");
		action.id="24";


		// merge MarginsBorderEdge -> Scintillas
		action = pushActionByPathFieldedIDForFieldInTag(null, MERGE, "id", "6293", "name", "Item", "Native-Langue", "Dialog", "Preference", "Scintillas");
		action.XMLPath1 = new String[]{"Native-Langue", "Dialog", "Preference", "MarginsBorderEdge"};

		// Line Number
		action = pushActionByPathFieldedIDForFieldInTag(null, DELETE, "id", "6291", "name", "Item", "Native-Langue", "Dialog", "Preference", "Scintillas");
		//action = pushActionByPathFieldedIDForFieldInTag(null, DELETE, "id", "6206", "name", "Item", "Native-Langue", "Dialog", "Preference", "Scintillas");
		action = pushActionByPathFieldedIDForFieldInTag(null, DELETE, "id", "6292", "name", "Item", "Native-Langue", "Dialog", "Preference", "Scintillas");
		action = pushActionByPathFieldedIDForFieldInTag(null, DELETE, "id", "6293", "name", "Item", "Native-Langue", "Dialog", "Preference", "Scintillas");


//Display line number
		action = pushActionByPathFieldedIDForFieldInTag("{sq:'Shfaq numrat e linjave', ar:'عرض رقم السطر', az:'Sətirlərin nömrələrinin göstər', ga:'Display line number', et:'Näita reanumbreid', eu:'Erakutsi lerro zenbakiak', be:'Паказваць нумар радка', bg:'Показване номер на реда', pl:'Pokazuj numer linii', bs:'Prikaži marginu za broj linije', af:'Wys Lyn nommer margine', da:'Vis linjenummer', de:'Zeilennummernrand', ru:'Отображать Нумерацию Строк', fr:'Afficher la numérotation des lignes', fi:'Näytä rivinumerot', ka:'ხაზების დანომრვა', gu:'લાઈન નંબર બતાવો', kk:'Жол нөмірі аумағын көрсету', ko:'줄 번호 표시', nl:'Regelnummers weergeven', ky:'Сап номурун көрсөт', gl:'Amosar número de liña', ca:'Mostra el número de línia al marge', cs:'Zobrazit okrajový pruh s čísly řádků', kn:'ಡಿಸ್‌ಪ್ಲೇ ಲೈನ್ ನಂಬರ್ಸ್', co:'Affissà u numeru di linea', hr:'Prikaz ruba s brojem retka', ku:'Display line numbers', lv:'Rādīt rindu numurus', lt:'Rodyti eilutės numerį', lb:'Zeilennummerenrand', ro:'Afișare numere linii', mr:'लाईन नंबर दाखवा', ms:'Paparkan Jidar nombor baris', mk:'Прикажи ја ивицата на бројот на линијата', mn:'Мөрний дугаарыг харуулах', bn:'দেখান লাইন সংখ্যা', zu:'Bonisa inombolo yomugqa', ne:'लाइन संख्या प्रदर्शन गर्नुहोस्', no:'Vis linjenummer', pa:'ਡਿਸਪਲੇਅ ਲਾਈਨ ਨੰਬਰ', pt:'Mostrar número das linhas', ja:'行番号を表示', sv:'Visa radnummer', sr:'Prikaži marginu za broj linije', si:'තීරු අංක පෙන්වන්න', eo:'Montru numerojn de linioj', sk:'Zobraziť čísla riadkov', sl:'Prikaži številko vrstice', tg:'Намоиши шумориши хатҳо', te:'డిస్‌ప్లే లైన్ నంబర్స్', ta:'வரி எண்களை காட்டு', th:'แสดงหมายเลขบรรทัด', tr:'Satır numarasını görüntüle', cy:'Dangos rhifau llinellau', uk:'Показувати номер рядка', uz:'Satrlar Raqamini ko‘rsatmoq', es:'Mostrar números de línea', el:'Εμφάνιση αριθμού Γραμμών περιθωρίων', hu:'Sorok számozása', it:'Visualizza i numeri di riga', hi:'लाईन नंबर दिखायें', id:'Tampilkan nomor baris', en:'Display line number', en1:'Display line number', vi:'Hiển thị số dòng', zh_CN:'显示行号栏', arg:'Amostrar o numero de linia a o marguin', bre:'Diskouez niverennoù al linennoù', fri:'Mostre numars di rie sul margjin', kab:'Beqqeḍ utunen n ijerriḍen', nno:'Vis linjenummer', oci:'Mostrar los numèros de linha', sar:'Faghe a bìdere is nùmeros de riga', venetian:'Visuaƚixa i nùmari de riga', aranese:'Ensenahar es nombres de linha en eth marge', extremaduran:'Muestral númirus de línia\\'n marhin', farsi:'نمایش شماره خطوط', ligurian:'Vixoalizza i numeri de riga', piglatin:'Isplayday inelay umbernay', serbianCyrillic:'Прикажи број реда', spanish_ar:'Mostrar número de línea', tagalog:'Ipakita ang margin ng Bilang ng taludtod', taiwanese:'顯示行號', tatar:'Юллар Санын күрсәтергә', uyghur:'قۇر نومۇرى كۆرسىتىش ', uz_b:'Сатр рақамлари майдонини кўрсатиш', pt_b:'Exibir número das linhas'}", MODIFY, "id", 6206, "name", "Item", "Native-Langue", "Dialog", "Preference", "Scintillas");

//Multi-Editing Settings
		action = pushActionByPathFieldedIDForFieldInTag("{sq:'Rregullimet Multi-korrigjim', ar:'إعدادات التعديل المتعدد', az:'Multi-Redaktə nizamları', ga:'Multi-Editing Settings', et:'Mitmik-redigeerimise sätted', eu:'Editatze-Anitz Ezarpenak', be:'Налады мультырэдагавання', bg:'Множествено редактиране', pl:'Edycja wieloliniowa', da:'Multi-redigeringsindstillinger', de:'Mehrfaches Editieren', ru:'Мультиредактирование', fr:'Multi-édition', fi:'Monimuokkaus', ka:'მრავალი ჩასწორების რეჟიმი', gu:'મલ્ટી-એડીટીંગ સેટીંગઝ', ko:'다중 편집 설정', nl:'Meervoudig bewerken', ky:'Мульти-оңдоо', ca:'Multi-edició', cs:'Vícenásobná editace', kn:'ಮಲ್ಟೀ-ಎಡಿಟಿಂಗ್ ಸೆಟ್ಟಿಂಗ್ಸ್', co:'Preferenze di multi-mudificazione', hr:'Postavke višestrukog uređivanja', ku:'Multi-Editing Settings', lv:'Uzlabotās rediģēšanas iestatījumi', lt:'Daugybinis redagavimas', ro:'Setări editare multiplă', mr:'बहु-संपादन संरचना', mn:'Давхар-Засах тохиргоо', bn:'মাল্টি সম্পাদনা সেটিংস', zu:'Izilungiselelo eziningi zokuhlela', ne:'बहु-सम्पादन सेटिंग्स', pa:'ਮਲਟੀ-ਸੋਧ ਸੈਟਿੰਗ', pt:' Definições da Edição Múltipla', ja:'一括編集', sv:'Inställningar för multiredigering', si:'බහු සංස්කාරක සිටවුම්', eo:'Agordoj pri plurredaktado', sk:'Nastavenie viacnásobnej editácie', sl:'Nastavitve kombiniranega urejanja', tg:'Танзимоти чанд-вироишӣ', te:'మల్టీ-ఎడిటింగ్ సెట్టింగ్స్', ta:'பற்-பதிப்பு அமைப்பு', th:'ตั้งค่าหลายการแก้ไข', tr:'Çoklu Düzenleme Ayarları', cy:'Gosodiadau Aml-golygu', uk:'Налаштування мульти-редагування', uz:'Multitahrirlamoq', es:'Multi-Edición', el:'Ρυθμίσεις Πολλαπλής Επεξεργασίας', hu:'Többszörös szerkesztés', it:'Selezione Multipla', hi:'मल्टी-एडिटिंग सेटिंग्स', id:'Pengaturan Multiedit', en:'Multi-Editing Settings', en1:'Multi-Editing Settings', vi:'Multi-Editing Settings', zh_CN:'多列编辑', arg:'Multi-ediciÃƒÂ³n', bre:'Arventennoù lies-aozañ', fri:'Impostazions multi editing', kab:'Multi-Editing Settings', nno:'Multiredigering', oci:'Paramètres de multi-edicion', sar:'Impostatziones sèberu mùltiplu', venetian:'Seƚesion Mùltipla', farsi:'تنظیمات چند-ویرایشی', ligurian:'Inpostaçioin seleçion multipla', piglatin:'Ultimay-Editingyay Ettingssay', serbianCyrillic:'Више уређивача', spanish_ar:'Multiedición', taiwanese:'多個位置同時編輯', tatar:'Мульти-үзгәртү', uyghur:'كۆپ ئىستونلۇق تەھرىرلەش', pt_b:'Multi-edição'}", INSERT, "id", 6224, "name", "Item", "Native-Langue", "Dialog", "Preference", "Scintillas");

//Enable (Ctrl+Mouse click/selection)
		action = pushActionByPathFieldedIDForFieldInTag("{sq:'Aftëso (Ctrl+klikim miu/zgjedhje)', ar:'تفعيل (Ctrl + الفأرة نقر / تحديد)', az:'İcazə ver (Ctrl+Mouse klik/Seçmək)', ga:'Cumasaigh (Ctrl+Mouse click/selection)', et:'Võimalda (Ctrl+hiireklikk/valik)', eu:'Gaitu (Ktrl+Xagu klika/hautapena)', be:'Укл. (Ctrl+націск мышы/выбар)', bg:'Включено (Ctrl + избор с мишката)', pl:'Włącz (Ctrl+LPM/zaznaczanie)', da:'Aktivér (Ctrl+klik/markering med mus)', de:'Aktivieren (Strg+Mausklick)', ru:'Вкл. (Ctrl + Клик / Выделение)', fr:'Activer (Ctrl+Clic/sélection)', fi:'Käytä (Ctrl+Napsaus/valinta)', ka:'ჩართვა (Ctrl+მარც. ღილაკი/მონიშვნა)', gu:'સક્રિય કરો (Ctrl+માઉસ ક્લીક/સિલેકશન)', ko:'가능 (Ctrl+마우스 클릭/선택)', nl:'Activeren', ky:'Ctrl+Чычкан чертүү/тандоо', ca:'Habilita (Ctrl+Ratolí)', cs:'Povolit (Ctrl+Myš(kliknutí/výběr))', kn:'ಎನೇಬಲ್ (ಕ್ಟ್ರ್ಲ್+ಮೌಸ್ ಕ್ಲಿಕ್/ಸೆಲೆಕ್ಶನ್)', co:'Attivà (Ctrl+Cliccu/Selez. cù Topu)', hr:'Omogući (Ctrl+Miš klik/označavanje)', ku:'Enable (Ctrl+Mouse click/selection)', lv:'Ieslēgt (Ctrl+peles klikšķis/iezīme)', lt:'Įgalinti (Ctrl+Pelės pažymėjimas)', ro:'Activare (Ctrl+clic maus pe selecție)', mr:'सक्रिय (Ctrl+Mouse click/selection)', mn:'Идэвхижүүлэх (Ctrl+Хулгана)', bn:'সক্রিয় করুন (Ctrl + মাউস ক্লিক / নির্বাচন)', zu:'Nika amandla (ctrl + chofoza umouse / ukukhetha)', ne:'सक्षम गर्नुहोस् (Ctrl + माउस क्लिक / चयन)', pa:'ਯੋਗ (Ctrl + ਮਾਉਸ ਨੂੰ ਕਲਿੱਕ ਕਰੋ / ਚੋਣ)', pt:'Ativar (Ctrl+ Clique Rato/seleção)', ja:'有効(Ctrl+クリック/選択)', sv:'Tillåt (Ctrl+Musklick/markering)', si:'සබල කරනවා (Ctrl+Mouse click/selection)', eo:'Ŝaltu (Ctrl+Musklako/Elektado)', sk:'Povoliť Ctrl+klik pre výber', sl:'Omogoči (Ctrl+klik miške/izbor)', tg:'Фаъолсозӣ (Ctrl+Mouse click/selection)', te:'ఎనేబల్ (కంట్రోల్+మౌస్ క్లిక్/సెలెక్షన్)', ta:'இயளுமைபடுத்து (Ctrl+Mouse click/selection)', th:'เปิดใช้ (Ctrl+Mouse click/selection)', tr:'Etkin (Ctrl + fareyle tıkla/seç)', cy:'Galluogi (Ctrl+Clic llygoden/dewis)', uk:'Увімк. (Ctrl+Клік миші/вибір)', uz:'Yoq. (Ctrl + Bosish / Ajratish)', es:'Habilitar (Ctrl. + clic/selección)', el:'Ενεργοποίηση (Ctrl+Κλικ Ποντικιού/επιλογή)', hu:'Engedélyezés (Ctrl+Egér/kijelölés)', it:'Abilita (Ctrl+clic/selezione)', hi:'सक्रिय (Ctrl+Mouse click/selection)', id:'Aktifkan (Ctrl+Mouse)', en:'Enable (Ctrl+Mouse click/selection)', en1:'Enable (Ctrl+Mouse click/selection)', vi:'Enable (Ctrl+Mouse click/selection)', zh_CN:'启用 (Ctrl+鼠标点选)', arg:'Habilitar (Ctrl+zorz)', bre:'Enaouiñ (Ktrl+Klik diuzañ)', fri:'Ative (Ctrl+Clic dal mouse/selezion)', kab:'Enable (Ctrl+Mouse click/selection)', nno:'Bruk (Ctrl+museklikk/merking)', oci:'Activar (Ctrl + clic/seleccion)', sar:'Abìlita (Ctrl+Click/seletzione)', venetian:'Abìƚita (Ctrl+clic/seƚesion)', farsi:'فعال کردن (Ctrl+کلیک ماوس/انتخاب)', ligurian:'Abilita (Ctrl+Click/seleçion)', piglatin:'Enableyay (Ctrl+mouse ickclay/electionsay)', serbianCyrillic:'Омогући (Ctrl+клик мишем/избор)', spanish_ar:'Habilita (Ctrl+Ratón)', taiwanese:'啟用（Ctrl + 滑鼠點擊/選取）', tatar:'Каб. (Ctrl + Тычкан чиртү/сайлау)', uyghur:'قوزغىتىش (Ctrl+Mouse click/selection)', pt_b:'Habilitar (Ctrl+Mouse)'}", MODIFY, "id", 6225, "name", "Item", "Native-Langue", "Dialog", "Preference", "Scintillas");

//Use new style dialog
		action = pushActionByPathFieldedIDForFieldInTag("{ar:'استخدام مربع حوار بالنمط الجديد ', ga:'Use new style dialog ', et:'Kasuta uues stiilis salvestamise dialoogi ', eu:'Erabili estilo berriko gordetze elkarrizketa ', be:'Выкарыстоўваць новы стыль дыялогу ', bg:'Нов диалогов стил  в прозореца за \"Запис като..\"', pl:'Używaj nowego okna zapisu ', da:'Brug gem-dialog med nyt udseende ', de:'Dialog im neuen Stil benutzen ', ru:'Новый диалог сохранения файла ', fr:'Activer le nouveau style de dialogue ', fi:'Käytä uutta tyyli-ikkunaa ', ko:'새로운 스타일로 대화 저장 ', nl:'Gebruik vensters nieuwe stijl ', ca:'Diàleg de desament sense capacitat de camí d\\'estil Unix', cs:'Použít nový styl dialogu ', co:'Impiegà u dialogu di stilu novu ', hr:'Upotrijebi novi dijalog stila ', ku:'Use new style save dialog ', lt:'Naudoti naują lango stilių', ro:'Utilizare stil nou dialog salvare ', zu:'Sebenzisaingxoxoyesitayelaesisha', ne:'नयाँ शैली संवाद प्रयोग गर्नुहोस् ', ja:'新しいファイルダイアログ', sv:'Använd moderna dialogrutor ', sk:'Použiť nový štýl dialógového okna ', sl:'Uporabi nov slog pogovornega okna ', tr:'Yeni stil diyalogunu kullan ', uk:'Використовувати новий стиль діалогу ', uz:'Faylni saqlashning yangi darchasi', es:'Usar nuevo estilo de diálogo Guardar ', hu:'Újabb típusú dialógus használata ', it:'Non selezionare estensione predefinita nella finestra di salvataggio', hi:'नए स्टाइल डायलॉग का प्रयोग करें ', id:'Gunakan dialog gaya baru ', en:'Use new style dialog ', en1:'Use new style dialog ', zh_CN:'使用新样式对话框', bre:'Implijout ar stil divizoù nevez ', oci:'Utilizar la nòva fenèstra d\\'enregistrement ', venetian:'Uxa novo stile de dialogo', farsi:'از پنجره ذخیره سبک جدید استفاده کن', piglatin:'Useyay ewnay ylestay ialogday', taiwanese:'使用新式對話方塊', tatar:'Файл саклауның яңа диалогы', pt_b:'Usar o novo estilo de diálogo Salvar'}", MODIFY, "id", 6430, "name", "Item", "Native-Langue", "Dialog", "Preference", "DefaultDir");

//finder-copy
		action = pushActionByPathFieldedIDForFieldInTag("{ar:'نسخ', ga:'Cóipeáil', be:'Капіяваць', bg:'Копиране', pl:'Kopiuj', da:'Kopier', de:'Kopieren', ru:'Копировать', fr:'Copier', fi:'Kopioi', ko:'복사', nl:'Kopiëren', ca:'Copia', cs:'Kopírovat', co:'Cupià', hr:'Kopiraj', lt:'Kopijuoti', ro:'Copiere', zu:'Kopisha', ne:'कापी', ja:'コピー', sv:'Kopiera', sk:'Kopírovať', sl:'Kopiraj', tr:'Kopyala', uk:'Копіювати', uz:'Nusxalamoq', es:'Copiar', hu:'Másolás', it:'Copia', hi:'कॉपी करें', id:'Salin', en:'Copy', en1:'Copy', zh_CN:'复制', bre:'Eilañ', venetian:'Còpia', piglatin:'Opycay', taiwanese:'複製', pt_b:'Copiar'}", INSERT, null, null, "value", "Item", "Native-Langue", "MiscStrings", "finder-copy");

//Cloud
		action = pushActionByPathFieldedIDForFieldInTag("{sq:'Cloud', ar:'التخزين السحابي', ga:'Néal', et:'Pilv', eu:'Hodeia', be:'Воблачнае сховішча', bg:'Облак', pl:'Chmura', da:'Skyen', de:'Cloud-Einstellungen', ru:'Облачное Хранение', fr:'Cloud', fi:'Pilvipalvelut', ko:'클라우드', nl:'Online opslag', ca:'Núvol', cs:'Cloud (i-úložišťě)', kn:'Cloud', co:'Nivulu', hr:'Oblak', ku:'هەوری', lv:'Mākoņošana', lt:'Debesija', ro:'Cloud', mn:'Үүл', zu:'Ifu', ne:'क्लाउड', pa:'Cloud', pt:'Cloud', ja:'クラウド', sv:'Moln', sk:'Úložisko', sl:'Oblak', tg:'Захираи абрӣ', tr:'Bulut', cy:'Cwmwl', uk:'Хмарне сховище', uz:'Saqlama', es:'Nube', hu:'Felhő', it:'Cloud', hi:'क्लाउड (Cloud)', id:'Awan', en:'Cloud', en1:'Cloud', zh_CN:'云同步', bre:'Koumoulenn', venetian:'Cloud', farsi:'اَبری', piglatin:'Cloud', taiwanese:'雲端', tatar:'Болытлы Саклау', pt_b:'Nuvem'}", MODIFY, null, null, "title", "Item", "Native-Langue", "Dialog", "Preference", "Cloud");
		

		// move Cloud -> MISC
		int[] items = new int[]{6318, 6319, 6320, 6350};
		for(int id:items) {
			action = pushActionByPathFieldedIDForFieldInTag(null, MOVE, "id", id, "name", "Item", "Native-Langue", "Dialog", "Preference", "MISC");
			action.XMLPath1 = new String[]{"Native-Langue", "Dialog", "Preference", "Cloud"};
		}
		
		// URI customized schemes:
		action = pushActionByPathFieldedIDForFieldInTag(null, DELETE, "id", "6264", "name", "Item", "Native-Langue", "Dialog", "Preference", "Cloud");
		
		
		action = pushActionByPathFieldedIDForFieldInTag(null, DELETE, null, null, "name", "Item", "Native-Langue", "WindowsDlg", "NbDocsTotal");
		
		action = pushActionByPathFieldedIDForFieldInTag(null, DELETE, null, null, "name", "Item", "Native-Langue", "MiscStrings", "finder-copy-verbatim");


		action = pushActionByPathFieldedIDForFieldInTag(null, DELETE, null, null, "name", "Item", "Native-Langue", "MiscStrings", "session-save-folder-as-workspace");



		action = pushActionByPathFieldedIDForFieldInTag("{zh_CN:'复制完整路径(&F)'}", MODIFY, "CMID", 7, "name", "Item", "Native-Langue", "Menu", "TabBar");

		action = pushActionByPathFieldedIDForFieldInTag("{zh_CN:'复制完整路径(&F)'}", MODIFY, "CMID", 7, "name", "Item", "Native-Langue", "Menu", "TabBar");
		action = pushActionByPathFieldedIDForFieldInTag("{zh_CN:'复制文件名称'}", MODIFY, "CMID", 8, "name", "Item", "Native-Langue", "Menu", "TabBar");
		action = pushActionByPathFieldedIDForFieldInTag("{en:'Directory to Clipboard',zh_CN:'复制文件路径'}", MODIFY, "CMID", 9, "name", "Item", "Native-Langue", "Menu", "TabBar");
		action = pushActionByPathFieldedIDForFieldInTag("{zh_CN:'重命名…'}", MODIFY, "CMID", 10, "name", "Item", "Native-Langue", "Menu", "TabBar");
		action = pushActionByPathFieldedIDForFieldInTag("{zh_CN:'移至回收站'}", MODIFY, "CMID", 11, "name", "Item", "Native-Langue", "Menu", "TabBar");
		action = pushActionByPathFieldedIDForFieldInTag("{zh_CN:'打开所在文件夹(&O)'}", MODIFY, "CMID", 19, "name", "Item", "Native-Langue", "Menu", "TabBar");
		action = pushActionByPathFieldedIDForFieldInTag("{zh_CN:'打开所在命令行'}", MODIFY, "CMID", 20, "name", "Item", "Native-Langue", "Menu", "TabBar");
		
		


	}
}

package apply.translations;

import static apply.translations.TextrumentLocalePatchs.*;

public class Patch_V1 implements TextrumentLocalePatchs.ActionApplier {
	@Override
	public void pushActions() {
			Action action;
			actions.add(new TextrumentLocalePatchs.Action(null, 0, "Native-Langue", "Dialog", "ShortcutMapper", "MainCommandNames"));

			pushActionByPathIDForNameField("{"+ TextrumentLocalePatchs.LANG.English+":'Current File Path to Clipboard'}", MODIFY, 42029, "Native-Langue", "Menu", "Main", "Commands");
			pushActionByPathIDForNameField("{"+ TextrumentLocalePatchs.LANG.English+":'Current Filename to Clipboard'}", MODIFY, 42030, "Native-Langue", "Menu", "Main", "Commands");
			pushActionByPathIDForNameField("{"+ TextrumentLocalePatchs.LANG.English+":'Current Dir. Path to Clipboard'}", MODIFY, 42031, "Native-Langue", "Menu", "Main", "Commands");

			pushActionByPathIDForNameField("{"+ TextrumentLocalePatchs.LANG.ChineseSimplified+":'清除所有格式'}", MODIFY, 43032, "Native-Langue", "Menu", "Main", "Commands");

			pushActionByPathFieldedIDForNameField("{"+ TextrumentLocalePatchs.LANG.English+":'File Dir Path to Clipboard'}", MODIFY, "CMID", 9, "Native-Langue", "Menu", "TabBar");

			pushActionByPathFieldedIDForNameField("{"+ TextrumentLocalePatchs.LANG.ChineseSimplified+":'复制完整路径到剪切板'}", MODIFY, "CMID", 7, "Native-Langue", "Menu", "TabBar");
			pushActionByPathFieldedIDForNameField("{"+ TextrumentLocalePatchs.LANG.ChineseSimplified+":'复制文件路径到剪切板'}", MODIFY, "CMID", 9, "Native-Langue", "Menu", "TabBar");

			//打开快捷方式所指文件 ( Open Linked File / Directory) 
			action = pushActionByPathFieldedIDForNameField("{sq:'Të hapur dosjen e lidhur / Directory', ar:'فتح الملف المرتبط / دليل', am:'የተገናኘውን ፋይል ክፈት / ማውጫ', az:'bağlı faylını açın / Directory', ga:'Oscail an comhad nasctha / Eolaire', et:'Avage seotud faili / Directory', eu:'Ireki lotuta fitxategia / direktorioa', be:'Адкрыць звязаны файл / каталог', bg:'Отворете свързаният файл / директория', is:'Opnaðu tengda skrá / möppu', pl:'Otwórz połączony plik / katalog', bs:'Otvorite povezani datoteku / direktorij', fa:'باز کردن فایل مرتبط / راهنمای', af:'Maak die gekoppelde lêer / Directory', da:'Åbn den sammenkædede fil / Bibliotek', de:'Öffnen Sie die verknüpfte Datei / Verzeichnis', ru:'Открыть связанный файл / каталог', fr:'Ouvrez le fichier lié / Directory', tl:'Buksan ang naka-link na file / Directory', fi:'Avaa linkitetty tiedosto / hakemisto', fy:'Iepenje it keppele triem / Directory', km:'បើកឯកសារដែលបានតភ្ជាប់ / ថត', ka:'გახსნა უკავშირდება ფაილი / დირექტორია', gu:'કડી ફાઇલ ખોલો / ડિરેક્ટરી', kk:'байланыстырылған файл / каталогты ашыңыз', ht:'Louvri dosye a lye / Anyè', ko:'링크 된 파일을 엽니 다 / 디렉토리', ha:'Bude da nasaba fayil / Directory', nl:'Open het gekoppelde bestand / Directory', ky:'байланышкан билэни ачуу / Directory', gl:'Abre o arquivo vinculado / Directorio', ca:'Obrir l&#39;arxiu vinculat / Directori', cs:'Otevřete propojený soubor / adresář', kn:'ಸಂಬಂಧಿತ ಫೈಲ್‌ಗಳನ್ನು ತೆರೆಯಿರಿ / ಡೈರೆಕ್ಟರಿ', co:'Apre u cartulare ambiguitati / Directory', hr:'Otvorite povezani datoteku / direktorij', ku:'Open the file girêdayî / Directory', la:'Aperi coniunctum lima / Directory', lv:'Atveriet saistītu failu / direktoriju', lo:'ເປີດໄຟລ໌ທີ່ເຊື່ອມໂຍງ / Directory', lt:'Atidaryti susietą failą / katalogą', lb:'Open de verlinkte Fichier / Guide', ro:'Deschideți fișierul legat / Director', mg:'Sokafy ny mifandray rakitra / Directory', mt:'Iftaħ il-fajl marbut / Direttorju', mr:'लिंक फाइल उघडा / निर्देशिका', ml:'ലിങ്കുചെയ്ത ഫയൽ / ഡയറക്ടറി തുറക്കുക', ms:'Buka fail yang berkaitan / Direktori', mk:'Отворете ја оваа датотека / Директориум', mi:'Whakatūwhera te kōnae e hono ana / Directory', mn:'Нээх холбоотой файлыг / сан', bn:'লিঙ্ক ফাইল খুলুন / নির্দেশিকা', my:'ယင်းနှင့်ဆက်စပ်ဖိုင် / Directory ကိုဖွင့်ပါ', hmn:'Qhib lub txuas ntaub ntawv / Directory', xh:'Vula ifayile edityaniswe / Directory', zu:'Vula ifayela exhumene / Directory', ne:'लिङ्क गरिएको फाइल खोल्न / निर्देशिका', no:'Åpne den koblede filen / katalogen', pa:'ਨਾਲ ਜੋੜਿਆ ਫਾਇਲ ਨੂੰ ਖੋਲ੍ਹੋ / ਡਾਇਰੈਕਟਰੀ', pt:'Abra o arquivo vinculado / Diretório', ps:'اړوند فایل / فولډر خلاص کړئ', ny:'Tsegulani file zogwirizana / Directory', ja:'リンクされたファイル/ディレクトリを開きます。', sv:'Öppna den länkade filen / Directory', sm:'Tatala le faila fesootai / Directory', sr:'Отворите повезани фајл / Дирецтори', st:'Bula faele amahanngoa / Directory', si:'සම්බන්ධිත ගොනුව / නාමාවලිය විවෘත', eo:'Malfermu la ligitaj dosiero / Katalogo', sk:'Otvorte prepojený súbor / adresár', sl:'Odprite povezano datoteko / imenik', sw:'Kufungua faili wanaohusishwa / Directory', gd:'Fosgail an ceangal faidhle / Directory', ceb:'Ablihi ang nalambigit file / Directory', so:'Fur faylka lala / Directory', tg:'Кушодани файл алоқаманд / Directory', te:'లింక్ ఫైలు తెరవండి / డైరెక్టరీ', ta:'தொடர்புடைய கோப்பு / கோப்புறையைத் திறக்கவும்', th:'เปิดแฟ้มที่เชื่อมโยง / ไดเรกทอรี', tr:'bağlantılı dosyayı açın / Directory', cy:'Agorwch y ffeil cysylltiedig / Directory', ur:'منسلک فائل کو کھولیں / ڈائریکٹری', uk:'Відкрити пов&#39;язаний файл / каталог', uz:'bog&#39;liq faylni oching / Directory', es:'Abrir el archivo vinculado / Directorio', iw:'פתח את הקובץ המקושר / מדריך', el:'Ανοίξτε το συνδεδεμένο αρχείο / κατάλογο', haw:'E wehe i ka ua hoʻopili waihona / Papa Kuhikuhi', sd:'کولڻ جي جڙيل فائيل / فھرست', hu:'Nyissa meg a csatolt fájlt / Directory', sn:'Zarura yakabatana faira / Directory', hy:'Բացեք կապված ֆայլը / ՏԵՂԵԿԱՏՈՒ', ig:'Mepee jikọrọ file / Directory', it:'Aprire il file collegato / Directory', yi:'עפענען די לינגקט טעקע / דירעקטארי', hi:'लिंक की गई फ़ाइल खोलें / निर्देशिका', su:'Buka berkas numbu / Diréktori', id:'Buka file terkait / Directory', jw:'Bukak file disambung / Directory', en:'Open Linked File / Directory', yo:'Ṣii awọn ti sopọ mọ faili / Directory', vi:'Mở tập tin liên kết / Thư mục', zh_TW:'打開鏈接的文件/目錄', zh_CN:'打开快捷方式所指文件'}", INSERT, "CMID", 23, "Native-Langue", "Menu", "TabBar");
			action.tagName="Item";


			//填充选中的单词 ( Fetch selected word )
			pushActionByPathFieldedIDForNameField("{sq:'Fetch fjalën e zgjedhur', ar:'جلب الكلمة المحددة', am:'የተመረጠው ቃል ሰብስብ', az:'seçilmiş söz Fetch', ga:'Gabh focal roghnaithe', et:'Tõmba valitud sõna', eu:'Eskuratu hautatutako hitza', be:'Fetch абранага слова', bg:'Изважда избрана дума', is:'Ná valið orð', pl:'Pobrać wybrane słowo', bs:'Fetch odabrane riječi', fa:'واکشی کلمه انتخاب', af:'Haal gekies woord', da:'Hent valgte ord', de:'Fetch ausgewähltes Wort', ru:'Fetch выбранного слова', fr:'Fetch mot sélectionné', tl:'Fetch napiling salita', fi:'Nouda valittu sana', fy:'Helje selektearre wurd', km:'ទៅយកពាក្យដែលបានជ្រើស', ka:'მოიტანონ შერჩეული სიტყვა', gu:'પસંદ કરેલ શબ્દ મેળવો', kk:'Таңдалған сөзді алып', ht:'Chache chwazi mo', ko:'선택된 단어를 가져 오기', ha:'Dibo zaba kalma', nl:'Fetch geselecteerde woord', ky:'Келгиле тандалган сөздү', gl:'Fetch palabra seleccionada', ca:'Fetch paraula seleccionada', cs:'Načíst vybrané slovo', kn:'ಆಯ್ಕೆಮಾಡಿದ ಪದವನ್ನು ಪಡೆದುಕೊಳ್ಳಿ', co:'Pigliammi parolla sceltu', hr:'Fetch odabranu riječ', ku:'Fetch peyva hilbijartî', la:'Ut sermo ambiente', lv:'Ielādēt atlasīto vārdu', lo:'ມາດດຶງຂໍ້ມູນຄໍາທີ່ເລືອກ', lt:'Paduok pasirinktą žodį', lb:'Eriwergezunn ausgewielt Wuert', ro:'Preluați cuvântul selectat', mg:'Haka voafantina teny', mt:'Jġibu kelma magħżula', mr:'निवडक शब्द प्राप्त', ml:'തിരഞ്ഞെടുത്ത വാക്കും ലഭ്യമാക്കുക', ms:'Mengambil perkataan yang dipilih', mk:'Земи избраниот збор', mi:'Tiki kupu tīpakohia', mn:'сонгосон үгийг авч', bn:'নির্বাচিত শব্দ পান', my:'ရွေးချယ်ထားသည့်စကားလုံးဆွဲယူ', hmn:'Nqa xaiv lo lus', xh:'Landa igama ekhethiweyo', zu:'Landa izwi akhethiwe', ne:'चयन गरिएको शब्द ल्याउन', no:'Hent valgte ordet', pa:'ਚੁਣੇ ਸ਼ਬਦ ਪ੍ਰਾਪਤ', pt:'Fetch palavra selecionada', ps:'ټاکل کلمه راوړي', ny:'Kanditengere anasankha mawu', ja:'選択した単語を取得します', sv:'Hämta valda ord', sm:'Aumai upu filifilia', sr:'Фетцх изабрани реч', st:'Lata kgethile lentsoe', si:'තෝරාගත් වචනය බෝගයන්ගේ', eo:'Alportu elektitan vorton', sk:'Vybrať označené slovo', sl:'Pridobi izbrano besedo', sw:'Kuchota neno lililochaguliwa', gd:'Iarraidh a thaghadh facal', ceb:'Dad-a sa mga pinili nga pulong', so:'Keen erayga xulay', tg:'Чанд маротиба калимаи интихобшуда', te:'ఎంచుకున్న పదాన్ని పొందు', ta:'தேர்ந்தெடுத்த சொல்லையும் எடு', th:'Fetch คำที่เลือก', tr:'Seçilen kelime Getirme', cy:'Nôl gair a ddewiswyd', ur:'منتخب شدہ لفظ آوردہ', uk:'Fetch обраного слова', uz:'Tanlangan so&#39;zni olib', es:'Fetch palabra seleccionada', iw:'תביא שנבחרה מילה', el:'Λήψη επιλεγμένων λέξεων', haw:'Kiʻi i koho hua&#39;ōlelo', sd:'منتخب ڪلام آڻڻ', hu:'Lekérése kijelölt szót', sn:'Mundotora shoko yasarudzwa', hy:'Fetch ընտրված բառը', ig:'Ozi kpọtara họrọ okwu', it:'Fetch parola selezionata', yi:'ברענגען אויסגעקליבן וואָרט', hi:'चयनित शब्द लायें', su:'Dipulut Kecap dipilih', id:'Ambil kata yang dipilih', jw:'Gelanggang tembung milih', en:'Fetch selected word', yo:'Bu yan ọrọ', vi:'Fetch từ đã chọn', zh_TW:'提取選擇的字', zh_CN:'填充选中的单词'}", MODIFY, "id", 6901, "Native-Langue", "Dialog", "Preference", "Searching");
			
			//填充附近的单词 ( Get nearby words )
			action = pushActionByPathFieldedIDForNameField("{sq:'Get fjalë afërta', ar:'احصل على الكلمات القريبة', am:'በአቅራቢያ ያሉ ቃላት ያግኙ', az:'yaxın sözləri alın', ga:'Faigh focail in aice láimhe', et:'Hangi lähedal sõnad', eu:'Lortu inguruko hitzak', be:'Знайдзіце побач словы', bg:'Вземи близките думи', is:'Fá nágrenninu orð', pl:'Uzyskaj pobliskich słowa', bs:'Get obližnji riječi', fa:'مطلع کلمات در این نزدیکی هست', af:'Kry nabygeleë woorde', da:'Få nærliggende ord', de:'Erhalten Sie in der Nähe Worte', ru:'Получить близлежащие слова', fr:'Obtenir les mots à proximité', tl:'Kumuha ng mga kalapit na mga salita', fi:'Hanki lähellä olevia sanoja', fy:'Get buert wurden', km:'ទទួលយកពាក្យដែលនៅក្បែរនោះ', ka:'მიიღეთ მეზობელი სიტყვები', gu:'નજીકના શબ્દો મેળવો', kk:'сөздер Жақын алу', ht:'Jwenn mo ki tou pre', ko:'근처의 단어를 받기', ha:'Samun kusa kalmomi', nl:'Krijg nabijgelegen woorden', ky:'Get жакын сөздөр', gl:'Obter palabras próximas', ca:'Obtenir paraules properes', cs:'Získat nedaleké slova', kn:'ಹತ್ತಿರದ ಪದಗಳನ್ನು ಪಡೆಯಿರಿ', co:'Get e parolle quasi', hr:'Uzmite u blizini riječi', ku:'Get peyvên nêzî', la:'Ut verborum prope', lv:'Iegūt tuvumā vārdus', lo:'ໄດ້ຮັບຄໍາທີ່ໃກ້ຄຽງ', lt:'Gauk netoliese žodžiai', lb:'Kréien Emgéigend Wierder', ro:'Obțineți cuvinte din apropiere', mg:'Get teo akaiky teo teny', mt:'Get kliem fil-qrib', mr:'जवळपास शब्द मिळवा', ml:'നേടുക വാക്കുകൾ സമീപത്തുള്ള', ms:'Dapatkan fakta berdekatan', mk:'Добие во близина зборови', mi:'Haere kupu tata', mn:'ойролцоо үг авах', bn:'নিকটবর্তী শব্দ পান', my:'စကားများအနီးအနားမှာ Get', hmn:'Tau nyob ze lus', xh:'Fumana amagama ezikufutshane', zu:'Thola amagama aseduze', ne:'नजिकैको शब्दहरू प्राप्त', no:'Få nærliggende ord', pa:'ਨੇੜਲੇ ਸ਼ਬਦ ਲਵੋ', pt:'Obter palavras próximas', ps:'نږدې لغاتونه ترلاسه کړئ', ny:'Pezani mawu apafupi', ja:'近くの単語を取得します', sv:'Få närliggande ord', sm:'Ia maua upu lata ane', sr:'Гет оближње речи', st:'Fumana mantsoe a haufi', si:'ළඟ පහත වචන ලබා ගන්න', eo:'Akiri proksimaj vortoj', sk:'Získať neďalekej slová', sl:'Get bližini besede', sw:'Kupata maneno ya jirani', gd:'Faigh faisg air làimh faclan', ceb:'Get duol nga mga pulong', so:'Hel erayo dhow', tg:'Садо суханони ҳамсоя', te:'సమీపంలోని పదాలు పొందండి', ta:'அருகிலுள்ள வார்த்தைகள் பெற', th:'ได้รับคำที่ใกล้เคียง', tr:'Yakındaki kelimeleri alın', cy:'Cael geiriau cyfagos', ur:'قریبی الفاظ حاصل کریں', uk:'Отримати прилеглі слова', uz:'yaqin so&#39;zlarni qiling', es:'Obtener palabras cercanas', iw:'קבל מילים הסמוכים', el:'Πάρτε κοντά λέξεις', haw:'Kiʻi i kokoke hua&#39;ōlelo', sd:'لفظن جي ڀرسان حاصل', hu:'Get adott szó', sn:'Wana mashoko pedyo', hy:'Ստանալ մոտակա խոսքերը', ig:'Nweta dị nso okwu', it:'Get parole vicine', yi:'באַקומען נירביי ווערטער', hi:'आस-पास के शब्द प्राप्त करें', su:'Meunang kecap caket dieu', id:'Dapatkan kata-kata di dekatnya', jw:'Goleki tembung ing sacedhake', en:'Get nearby word', yo:'Gba wa nitosi ọrọ', vi:'Nhận lời lân cận', zh_TW:'獲取附近的單詞', zh_CN:'填充附近的单词'}", INSERT, "id", 6906, "Native-Langue", "Dialog", "Preference", "Searching");
			action.tagName="Item";
			
			
			//放大字体 ( Enlarge font size )
			action = pushActionByPathFieldedIDForNameField("{sq:'Enlarge font size', ar:'حجم الخط تكبير', am:'ያስረዝማሉ ቅርጸ-ቁምፊ መጠን', az:'Enlarge font size', ga:'Clómhéid Enlarge', et:'Suurenda kirja suurust', eu:'Zabaldu letra-tamaina', be:'Павялічыць памер шрыфта', bg:'размер на шрифта Увеличи', is:'Stækka leturstærð', pl:'Zwiększyć rozmiar czcionki', bs:'Povećaj veličinu slova', fa:'اندازه فونت زوم', af:'Vergroot lettergrootte', da:'Større skriftstørrelse', de:'Vergrößern Schriftgröße', ru:'Увеличить размер шрифта', fr:'Agrandir la police', tl:'laki ng font Lakhan', fi:'Isompi kirjasinkokoa', fy:'Enlarge font size', km:'ពង្រីកទំហំអក្សរ', ka:'Enlarge font size', gu:'વધારવું ફોન્ટ કદ', kk:'Үлкейту қаріп өлшемі', ht:'Laji font size', ko:'확대 글꼴 크기', ha:'Kara girma font size', nl:'Lettertype vergroten', ky:'Enlarge тексттин өлчөмү текст', gl:'Ampliar o tamaño da fonte', ca:'Augmentar la font', cs:'Zvětšit velikost písma', kn:'ದೊಡ್ಡದಕ್ಕಾಗಿ ಫಾಂಟ್ ಗಾತ್ರ', co:'Ingrandà font size', hr:'Povećaj veličinu slova', ku:'font size Enlarge', la:'Font magnitudine Terrarium Enlarge', lv:'Palielināt fonta izmērs', lo:'ຂະຫນາດ font ຂະຫຍາຍ', lt:'Padidinti šrifto dydį', lb:'Vergréisseren Schrëftgréisst', ro:'Mărește dimensiunea fontului', mg:'Halalaho haben&#39;ny endri-tsoratra', mt:'daqs tat-tipa tkabbar', mr:'मोठा फॉन्ट आकार', ml:'വലുതാക്കുന്നു ഫോണ്ട് വലുപ്പം', ms:'Besarkan saiz font', mk:'Зголеми големина на фонт', mi:'rahi momotuhi Kia rahi atu', mn:'Томруулж үсгийн хэмжээ', bn:'বৃদ্ধি করুন ফন্টের আকার', my:'Enlarge font size ကို', hmn:'Ntug font loj', xh:'Yandisa Ubungakanani bamagama', zu:'Khulisa usayizi wefonti', ne:'फन्ट आकार बढाउनुहोस्', no:'Større skriftstørrelse', pa:'ਵੱਡਾ ਫੋਟ ਦੇ ਅਕਾਰ', pt:'Alargar o tamanho da fonte', ps:'Enlarge font size', ny:'Phunzirani wosasintha kukula', ja:'拡大フォントサイズ', sv:'Förstora teckenstorlek', sm:'Faalautele tele vai', sr:'Енларге фонт сизе', st:'Suuremmaksi fonto', si:'විශාල ප්රමාණයේ අකුරු', eo:'Vastigi tiparo grandeco', sk:'Zväčšiť veľkosť písma', sl:'Povečaj velikost pisave', sw:'Kupanua ukubwa wa herufi', gd:'Enlarge font size', ceb:'gidak-on sa font Padakua', so:'Enlarge font size', tg:'Андозаи ҳарф рӯзии', te:'వచ్చేలా ఫాంట్ పరిమాణం', ta:'பெரிதாகும் எழுத்துரு அளவு', th:'ขนาดตัวอักษรขนาดใหญ่', tr:'Büyüt Yazı boyutunu', cy:'maint y ffont Enlarge', ur:'وسعت فونٹ سائز', uk:'Збільшити розмір шрифту', uz:'Soni o&#39;tish Shriftning o&#39;lchami', es:'Aumentar la fuente', iw:'גודל הגופן להגדיל', el:'Μεγέθυνση μέγεθος της γραμματοσειράς', haw:'A ho&#39;ākea Font nui', sd:'ڪشادي اکرن ۾ ماپ', hu:'Nagyobb betűméret', sn:'Enlarge Schriftgröße', hy:'Մեծացնել տառաչափը', ig:'Me font size', it:'Ingrandire la dimensione del carattere', yi:'פאַרגרעסערן שריפֿט גרייס', hi:'बड़े आकार में फ़ॉन्ट आकार', su:'Ukuran font ngalegaan', id:'ukuran font Enlarge', jw:'ukuran font Enlarge', en:'Enlarged font', yo:'Tobi font iwọn', vi:'kích thước font Enlarge', zh_TW:'放大字體', zh_CN:'放大字体'}", INSERT, "id", 6905, "Native-Langue", "Dialog", "Preference", "Searching");
			action.tagName="Item";
			
			//等宽字体 ( Monospaced Font )
			pushActionByPathFieldedIDForNameField("{sq:'Monospaced Font', ar:'أحادي المسافة الخط', am:'ባለጭረት እኩል ቅርጸ ቁምፊ', az:'orantısız Font', ga:'monospaced Cló', et:'Püsisammuga Font', eu:'tarte bakarreko Font', be:'монашырынны шрыфт', bg:'Непропорционални шрифта', is:'einföldu bili Font', pl:'Czcionka o stałej szerokości', bs:'monospaced Font', fa:'تک فاصلهدار فونت', af:'Enkelgespasieerde Font', da:'proportional skrifttype', de:'dicktengleiche Font', ru:'моноширинный шрифт', fr:'monospaced police', tl:'monospaced Font', fi:'kiinteävälinen Font', fy:'Monospaced Lettertype', km:'ដកឃ្លាពុម្ពអក្សរ', ka:'monospaced Font', gu:'Monospaced ફોન્ટ', kk:'Біркелкі шрифт', ht:'Monospaced Font', ko:'고정 폭 글꼴', ha:'Monospaced Font', nl:'enkelspatielettertype', ky:'Бирдей Арип', gl:'monospaced Font', ca:'font monospaced', cs:'Neproporcionální písmo', kn:'monospaced ಫಾಂಟ್', co:'Monospaced Font', hr:'Monospaced slova', ku:'hevnavberî Font', la:'Monospaced Font', lv:'Vienplatuma Fonts', lo:'Monospace Font', lt:'monospaced Šrifto', lb:'Monospaced Font', ro:'Monospaced Font', mg:'Monospaced Font', mt:'Monospaced Font', mr:'Monospaced फॉन्ट', ml:'ഏകാനുപാതത്തിലുള്ള ഫോണ്ട്', ms:'monospaced Font', mk:'monospaced фонт', mi:'Monospaced Momotuhi', mn:'Monospaced үсгийн', bn:'মোনোস্কেপ ফন্ট', my:'ဗြက်တူသောစာလုံး', hmn:'Monospaced Font', xh:'esinye somgca Font', zu:'Monospaced Ifonti', ne:'मोनोस्पेस फन्ट', no:'monospaced Font', pa:'ਮੋਨੋਸਪੇਸਡ Font', pt:'monospaced Font', ps:'Monospaced Font', ny:'Monospaced Zilembo', ja:'等幅フォント', sv:'Monospaced Font', sm:'Monospaced Faatanoa', sr:'Го Фонт фиксне ширине', st:'Monospaced Font', si:'Monospaced අකුරු', eo:'samspacitajn Tiparo', sk:'neproporcionálne písmo', sl:'manjšo pisavo', sw:'Monospaced Font', gd:'Monospaced Font', ceb:'Monospaced Font', so:'Monospaced Font', tg:'Monospaced ҳарф', te:'Monospaced ఫాంట్', ta:'அகலம் மாறா எழுத்துரு', th:'พิมพ์ดีดอักษร', tr:'Eş Aralıklı Yazı', cy:'unlled Font', ur:'monospaced فونٹ', uk:'моно шрифт', uz:'nomutanosib yozuv', es:'Fuente monospaced', iw:'Monospaced גופן', el:'monospaced γραμματοσειράς', haw:'Monospaced Font', sd:'Monospaced اکرن', hu:'proporcionális betűtípus', sn:'Monospaced Font', hy:'Monospaced Տառատեսակի', ig:'Monospaced Font', it:'Monospaced Font', yi:'מאָנאָספּאַסעד שריפֿט', hi:'Monospaced फ़ॉन्ट', su:'Monospaced Font', id:'monospace Font', jw:'Monospaced Font', en:'Monospaced font', yo:'Monospaced Font', vi:'monospaced Font', zh_TW:'等寬字體', zh_CN:'等宽字体'}", MODIFY, "id", 6902, "Native-Langue", "Dialog", "Preference", "Searching");
	

			//标签列表 ( Tab List )
			action = pushActionByPathFieldedIDForNameField("{sq:'Tab List', ar:'قائمة التبويب', am:'የትር ዝርዝር', az:'Tab siyahısı', ga:'Tab Liosta', et:'Tab loetelu', eu:'Fitxa zerrenda', be:'спіс Tab', bg:'Tab Списък', is:'Flipalisti', pl:'Zakładka Lista', bs:'Tab List', fa:'فهرست برگه', af:'blad Lys', da:'Tab List', de:'Registerliste', ru:'Список Tab', fr:'onglet Liste', tl:'Listahan ng tab', fi:'Välilehtiluettelo', fy:'Tab List', km:'បញ្ជីផ្ទាំង', ka:'tab სია', gu:'ટેબ સૂચિ', kk:'Tab тізімі', ht:'Contenu Lis', ko:'탭 목록', ha:'tab List', nl:'tab Lijst', ky:'өтмөк тизмеси', gl:'lista tab', ca:'llista pestanya', cs:'Karta Seznam', kn:'ಟ್ಯಾಬ್ ಪಟ್ಟಿ', co:'List Tab', hr:'kartica popis', ku:'Lîsteya tab', la:'List tab', lv:'Tab saraksts', lo:'ບັນຊີ tab', lt:'skirtukas sąrašas', lb:'Tab Lëscht', ro:'Tab Listă', mg:'Tab List', mt:'tab Lista', mr:'टॅब यादी', ml:'ടാബ് പട്ടിക', ms:'Senarai tab', mk:'Tab Листа', mi:'Rārangi ripa', mn:'Tab жагсаалт', bn:'ট্যাব তালিকা', my:'tab ကိုစာရင်း', hmn:'tab Sau', xh:'tab List', zu:'Ithebhu Yohlu', ne:'ट्याब सूची', no:'Tab List', pa:'ਟੈਬ ਸੂਚੀ', pt:'Lista tab', ps:'Tab بشپړفهرست', ny:'List Tab', ja:'タブの一覧', sv:'Tab Lista', sm:'Tab Lisi', sr:'Листа картица', st:'List tab ya', si:'tab ලැයිස්තුව', eo:'langeton Listo', sk:'karta Zoznam', sl:'zavihek Seznam', sw:'Orodha tab', gd:'tab List', ceb:'List tab', so:'tab List', tg:'Рӯйхати ҷадвалбандии', te:'టాబ్ జాబితా', ta:'தாவல் பட்டியல்', th:'แท็บรายการ', tr:'Sekme listesi', cy:'Rhestr tab', ur:'ٹیب کی فہرست', uk:'список Tab', uz:'Tab ro&#39;yxati', es:'Lista pestaña', iw:'Tab רשימה', el:'Λίστα tab', haw:'uku pila List', sd:'ٽئب لسٽ', hu:'Tab List', sn:'Tab List', hy:'tab ցուցակ', ig:'tab List', it:'Lista Tab', yi:'קוויטל רשימה', hi:'टैब सूची', su:'tab Daptar', id:'tab Daftar', jw:'tab List', en:'Tab List', yo:'Tab List', vi:'tab Danh sách', zh_TW:'選項卡列表', zh_CN:'标签页列表'}", INSERT, "id", 44070, "Native-Langue", "Menu", "Main", "Commands");
			action.tagName="Item";
			
			
			//切换至而不是切换面板 ( Activate the panel but do not toggle it )
			action = pushActionByPathFieldedIDForNameField("{sq:'Aktivizoni panelin, por nuk e bëjnë toggle atë', ar:'تفعيل لوحة ولكن لا تبديل ذلك', am:'የ የፓነል አግብር ግን ቀያይር አይደለም ማድረግ', az:'panel aktivləşdir ancaq keçid yoxdur', ga:'Gníomhachtaigh an bpainéal ach ní scoránaigh sé', et:'Aktiveeri paneel kuid ei lülita see', eu:'Aktibatu panela baina ez du ezer egingo toggle', be:'Актывуйце панэль, але не пераключыць яго', bg:'Активиране на панела, но не го превключване', is:'Virkjaðu spjaldið en ekki skipta henni', pl:'Aktywuj panel ale nie przestawić go', bs:'Aktivirajte panela ali ne toggle to', fa:'پانل فعال اما ضامن آن نیست', af:'Aktiveer die paneel maar doen nie skakel dit', da:'Aktiver panel, men ikke skifte det', de:'Aktivieren Sie das Panel aber nicht Toggle es', ru:'Активируйте панель, но не переключить его', fr:'Activez le panneau, mais ne pas choisir de l&#39;afficher', tl:'I-activate ang panel ngunit hindi magpalipat-lipat ito', fi:'Aktivoi paneelin mutta älä tasatuntisignaalin', fy:'Aktivearje it paniel, mar dogge it net Toggle it', km:'ធ្វើឱ្យបន្ទះនេះតែកុំបិទបើកវា', ka:'გააქტიურება panel მაგრამ არ toggle ეს', gu:'પેનલ સક્રિય પરંતુ તે ટૉગલ નથી', kk:'панелін іске қосыңыз, бірақ оны ауыстырыңыз емес,', ht:'Aktive panèl la men pa fè sa activer li', ko:'패널을 활성화하지만 토글하지 않습니다', ha:'Kunna panel amma ba toggle shi', nl:'Activeer het paneel maar niet toggle is', ky:'панелди иштетүү бирок которгуч эмес,', gl:'Activa o panel, pero non facelo de alternancia', ca:'Activa el panell però no ho fan de palanca es', cs:'Aktivovat panel, ale ne přepnout ním', kn:'ಫಲಕ ಸಕ್ರಿಯಗೊಳಿಸಿ ಆದರೆ ಕಂಬಿ ಹಾಗೆ', co:'Attivà panel, ma ùn fà Calmont lu', hr:'Aktivirajte ploču, ali ne preklopni toga', ku:'di panelê de aktîv bike di heman demê de do toggle ew ne', la:'Sed non eu in panel quod toggle', lv:'Aktivizēt paneli, bet ne pārslēgšanas to', lo:'ເປີດນໍາໃຊ້ຄະນະກໍາມະແຕ່ບໍ່ຫຼັບມັນ', lt:'Aktyvuoti skydą, bet ne perjungti jį', lb:'Aktivéieren de Rot mee nët toggle et', ro:'Activează panoul de comutare, dar nu-l', mg:'Mampihetsika ny tontonana fa aza Toggle izany', mt:'Jattiva-panel, imma ma toggle dan', mr:'पॅनल सक्रिय पण तो टॉगल नाही', ml:'പാനൽ സജീവമാക്കുക എന്നാൽ ടോഗിൾ ചെയ്യാൻ', ms:'Mengaktifkan panel tetapi tidak togol ia', mk:'Активирајте панелот но не го смениш', mi:'Whakahohe i te rōpū, engari e kore e meatia e takahuri reira', mn:'самбар идэвхжүүлэх гэхдээ түлхүүр байхгүй бол', bn:'প্যানেল সক্রিয় কিন্তু এটা টগল না', my:'အဆိုပါ panel ကိုကိုသက်ဝင်ဒါပေမယ့် Toggle အားမ', hmn:'Qhib lub vaj huam sib luag, tiam sis ua tsis tau toggle nws', xh:'Yenza iqela lenjongo kodwa musa lokutshintsha iimo kuyo', zu:'Yenza kusebenze iphaneli kodwa musa yokuguquguqula ke', ne:'प्यानल सक्रिय तर यो टगल छैन', no:'Aktiver panel men ikke veksle det', pa:'ਪੈਨਲ ਨੂੰ ਸਰਗਰਮ ਕਰੋ, ਪਰ ਇਸ ਨੂੰ ਟੌਗਲ ਨਾ ਕਰਦੇ', pt:'Ative o painel, mas não fazê-lo de alternância', ps:'ويناوالو فعالول خو څرنګتياونج دا نه', ny:'Gwiritsani ndi gulu koma osati toggle izo', ja:'パネルをアクティブにしますがトグルしません', sv:'Aktivera panelen men inte växla det', sm:'Faatoaaga le laulau ae faia le toggle ai', sr:'Активирајте панел, али га не искључи', st:'Ebe o kenya tshebetsong sehlopha empa le se ke fapoha ho', si:'මණ්ඩලයක් ක්රියාත්මක නමුත් එය ටොගල නැහැ', eo:'Aktivigi la panelo sed ne ebligi ŝin', sk:'Aktivovať panel, ale nie prepnúť ním', sl:'Aktivirajte ploščo, vendar ne preklopi njo', sw:'Kuamsha jopo lakini hawana kugeuza ni', gd:'&#39;Chur air a&#39; phannal ach chan eil e toglachaidh', ceb:'Activate sa panel apan dili toggle niini', so:'Dhaqaajiso guddiga laakiin aan samayn toggle', tg:'панел фаъол созед, вале корро иваз кардан на он', te:'ప్యానెల్ సక్రియం కానీ అది టోగుల్ లేదు', ta:'குழு செயல்படுத்தவும் ஆனால் அது மாற்று இல்லை', th:'เปิดใช้งานแผง แต่ไม่สลับ', tr:'panelini etkinleştirme ama geçiş yok', cy:'Activate y panel, ond nid yn ei wneud toggle ei', ur:'پینل چالو کریں لیکن یہ ٹوگل ایسا نہیں کرتے', uk:'Активуйте панель, але не переключити його', uz:'panel faollashtirish, lekin u tahrir yo&#39;q', es:'Activar el panel pero no lo hacen de palanca se', iw:'הפעל את הפאנל אבל לא לעבור אותו', el:'Ενεργοποιήστε το πλαίσιο, αλλά δεν εναλλαγής είναι', haw:'Haʻalele i ka panel akā, hana i holo waena mea', sd:'پينل ايڪٽيويٽ پر ان کي ٻٽو نه ڪندا', hu:'Aktiválja a panel, de nem tehetjük meg', sn:'Activate pemapuranga asi musaita Toggle izvozvo', hy:'Ակտիվացրեք վահանակը, բայց չեն փոխանջատիչ դրա', ig:'Rụọ ọrụ panel ma unu toggle ya', it:'Attivare il pannello, ma non farlo ginocchiera essa', yi:'אַקטאַווייט די טאַפליע אָבער טאָן ניט טאַגאַל עס', hi:'पैनल को सक्रिय करें लेकिन यह टॉगल नहीं है', su:'Aktipkeun panel tapi ulah toggle eta', id:'Aktifkan panel tapi tidak beralih itu', jw:'Aktifake panel nanging ora pilihan iku', en:'Switch To Instead of Toggle Panels', yo:'Mu awọn panel ṣugbọn se ko toggle o', vi:'Kích hoạt bảng điều khiển nhưng không chuyển đổi nó', zh_TW:'激活面板，但不切換它', zh_CN:'切换到而不是切换面板'}", INSERT, "id", 44105, "Native-Langue", "Menu", "Main", "Commands");
			action.tagName="Item";
			
			
			//工程面板 ( Project Panels )
			pushActionByPathFieldedIDForNameField("{sq:'Kolegjet e projektit', ar:'لوحات المشروع', am:'የፕሮጀክት ፓናሎች', az:'Layihə Panels', ga:'Painéil Tionscadail', et:'projekti Paneelid', eu:'Project panelak', be:'панэлі праекта', bg:'Панели на проекта', is:'Project Pallborð', pl:'Panele projektu', bs:'projekt Paneli', fa:'پانل های پروژه', af:'projek Panele', da:'Projekt Paneler', de:'Projekt Panels', ru:'Панели проекта', fr:'Panneaux projet', tl:'Project Panel', fi:'projekti Paneelit', fy:'Project Panielen', km:'គណៈកម្មាការគម្រោង', ka:'პროექტის პანელები', gu:'પ્રોજેક્ટ પેનલ્સ', kk:'Жоба панельдер', ht:'pwojè Panneaux', ko:'프로젝트 패널', ha:'Project bangarori', nl:'project panels', ky:'Долбоор Panels', gl:'Paneis de proxecto', ca:'Els panells de projectes', cs:'Panely projektu', kn:'ಪ್ರಾಜೆಕ್ಟ್ ಫಲಕಗಳನ್ನು', co:'Mino Project', hr:'Paneli Projekt', ku:'Panelan Project', la:'Project Panel', lv:'Projekta paneļi', lo:'Panels Project', lt:'projekto Plokštės', lb:'Projet Brieder', ro:'Panouri de proiect', mg:'Project Panels', mt:'Bordijiet proġett', mr:'प्रकल्प पॅनेल', ml:'പദ്ധതി പാനലുകൾ', ms:'Panel projek', mk:'панели', mi:'Panels kaupapa', mn:'Төслийн самбар', bn:'প্রকল্প প্যানেল', my:'Project မှ Panel များ', hmn:'Project Panels', xh:'Panel Project', zu:'Project Iphaneli', ne:'परियोजना पैनलों', no:'Prosjekt Panels', pa:'ਪ੍ਰੋਜੈਕਟ ਪੈਨਲ', pt:'Painéis de projeto', ps:'د پروژې پانل', ny:'mapanelo Project', ja:'プロジェクトパネル', sv:'Projekt paneler', sm:'Panels galuega', sr:'Пројецт Панели', st:'diphanele Project', si:'ව්යාපෘති මඩුලු', eo:'projekto Paneloj', sk:'panely projektu', sl:'Plošče projekta', sw:'Paneli Project', gd:'Pròiseact pannalan', ceb:'Project Panel', so:'Guddiyada Project', tg:'Гурӯҳҳои лоиња', te:'ప్రాజెక్టు ప్యానెల్లు', ta:'திட்ட பேனல்களை', th:'แผงโครงการ', tr:'Proje Paneller', cy:'Paneli prosiect', ur:'پروجیکٹ پینلز', uk:'панелі проекту', uz:'Project taxtali', es:'Los paneles de proyectos', iw:'פנל פרויקט', el:'Πίνακες Έργων', haw:'Project Panel', sd:'منصوبي پينل', hu:'Project panelek', sn:'Project ukomba', hy:'Ծրագրի Վահանակներ', ig:'Project ogwe', it:'Pannelli di progetto', yi:'פּראָיעקט פּאַנעלס', hi:'परियोजना पैनलों', su:'proyék Panels', id:'proyek Panel', jw:'project Panels', en:'Project Panels', yo:'Project Panels', vi:'Panels dự án', zh_TW:'工程面板', zh_CN:'工程面板'}", MODIFY, "subMenuId", "view-project", "Native-Langue", "Menu", "Main", "SubEntries");
			
			
			
			//小图标
			action = pushActionByPathFieldedIDForNameField("{sq:'Ikona të vogla', ar:'أيقونات صغيرة', az:'Balaca ikonlar', ga:'Deilbhíní móra', et:'Väikesed ikoonid', eu:'Ikur txikiak', be:'Маленькія значкі', bg:'Малки икони', pl:'Małe ikony', bs:'Male ikone', af:'Klein ikone', da:'Små ikoner', de:'Kleine Icons', ru:'Мелкие иконки', fr:'Petites icônes', fi:'Pienet kuvakkeet', ka:'პატარა ხატულები', gu:'નાના આઇકોન્સ', kk:'Ұсақ белгішелер', ko:'작은 아이콘', nl:'Klein', ky:'Майда белгичелер', gl:'Iconas pequenas', ca:'Icones petites', cs:'Malé ikony', kn:'ಸ್ಮಾಲ್ ಐಕಾನ್ಸ್', co:'Icone chjuche', hr:'Male ikone', ku:'ئایكۆنی بچووك', lv:'Mazās ikonas', lt:'Mažos piktogramos', lb:'Kleng Icons', ro:'Pictograme mici', mr:'छोटे आइकॉनस', ms:'Arca kecil', mk:'Мали икони', mn:'Жижиг дүрс', bn:'ছোট আইকন', zu:'Izithonjana ezincane', ne:'सानो आइकनहरू', no:'Små ikoner', pa:'ਛੋਟੇ ਆਈਕਾਨ', pt:'Ícones Pequenos', ja:'小さなアイコン', sv:'Små ikoner', sr:'Male ikone', si:'කුඩා නිරූපක ', eo:'Malgrandaj bildsimboloj', sk:'Malé ikony', sl:'Majhne ikone', te:'స్మాల్ ఐకాన్స్', ta:'சிறு படவுறு', th:'ไอคอนขนาดเล็ก', tr:'Küçük simgeler', cy:'Eiconau bach', uk:'Дрібні значки', uz:'Кичик белгилар', es:'Iconos pequeños', el:'Μικρά εικονίδια', hu:'Kis ikonok', it:'Icone piccole', hi:'छोटे आइकॉनस', id:'Ikon kecil', en:'Small icons', vi:'Biểu tượng nhỏ', zh_CN:'小图标'}", INSERT, "id", 48100, "Native-Langue", "Menu", "Main", "Commands");
			action.tagName="Item";
			
			//大图标
			action = pushActionByPathFieldedIDForNameField("{sq:'Ikona të mëdha', ar:'أيقونات كبيرة', az:'Böyük ikonlar', ga:'Deilbhíní beaga', et:'Suured ikoonid', eu:'Ikur handiak', be:'Вялікія значкі', bg:'Големи икони', pl:'Duże ikony', bs:'Velike ikone', af:'Groot ikone', da:'Store ikoner', de:'Große Icons', ru:'Крупные иконки', fr:'Grandes icônes', fi:'Isot kuvakkeet', ka:'დიდი ხატულები', gu:'મોટા આઇકોન્સ', kk:'Ірі белгішелер', ko:'큰 아이콘', nl:'Groot', ky:'Чоң белгичелер', gl:'Iconas grandes', ca:'Icones grans', cs:'Velké ikony', kn:'ಬಿಗ್ ಐಕಾನ್ಸ್', co:'Icone maiò', hr:'Velike ikone', ku:'ئایكۆنی گەورە', lv:'Lielās ikonas', lt:'Didelės piktogramos', lb:'Grouss Icons', ro:'Pictograme mari', mr:'मोठे आइकॉनस', ms:'Arca besar', mk:'Големи икони', mn:'Том дүрс', bn:'বড় আইকন', zu:'Izithombe ezinkulu', ne:'ठूला आइकनहरू', no:'Store ikoner', pa:'ਵੱਡੇ ਆਈਕਾਨ', pt:'Ícones Grandes', ja:'大きなアイコン', sv:'Stora ikoner', sr:'Velike ikone', si:'විශාල නිරූපක', eo:'Grandaj bildsimboloj', sk:'Veľké ikony', sl:'Velike ikone', te:'బిగ్ ఐకాన్స్', ta:'பெரு படவுறு', th:'ไอคอนขนาดใหญ่', tr:'Büyük simgeler', cy:'Eiconau mawr', uk:'Великі значки', uz:'Йирик белгилар', es:'Iconos grandes', el:'Μεγάλα εικονίδια', hu:'Nagy ikonok', it:'Icone grandi', hi:'बड़े आइकॉनस', id:'Ikon besar', en:'Big icons', vi:'Biểu tượng lớn', zh_CN:'大图标'}", INSERT, "id", 48101, "Native-Langue", "Menu", "Main", "Commands");
			action.tagName="Item";
			
			//标准图标
			action = pushActionByPathFieldedIDForNameField("{sq:'Ikona standarde', ar:'أيقونات قياسية', az:'Standart ikonlar', ga:'Deilbhíní caighdeánacha', et:'Tavasuuruses ikoonid', eu:'Ikur estandarrak', be:'Звычайныя значкі', bg:'Стандартни икони', pl:'Normalne ikony', bs:'Male standardne ikone', af:'Klein standaard ikone', da:'Standardikoner', de:'Kleine Standard-Icons', ru:'Стандартные иконки', fr:'Icônes standards', fi:'Normaalikuvakkeet', ka:'სტანდარტული ხატულები', gu:'સ્ટાનડર્દ આઇકોન્સ', kk:'Әдепкі белгішелер', ko:'표준 아이콘', nl:'Modern', ky:'Демейки белгичелер', gl:'Iconas pequenas predeterminadas', ca:'Icones estàndard', cs:'Standardní ikony', kn:'ಸ್ಟ್ಯಾಂಡರ್ಡ್ ಐಕಾನ್ಸ್', co:'Icone classiche', hr:'Male standardne ikone', ku:'ئایكۆنی ستاندارد', lv:'Parastas ikonas', lt:'Įprastinės piktogramos', lb:'Kleng Standard-Icons', ro:'Pictograme standard', mr:'आदर्श आइकॉन', ms:'Arca kecil piawai', mk:'Стандардни икони', mn:'Жишиг дүрс', bn:'মান আইকন', zu:'Izithonjana ezijwayelekile', ne:'मानक आइकनहरू', no:'Små standard-ikoner', pa:'ਮਿਆਰੀ ਆਈਕਾਨ', pt:'Ícones Normais', ja:'標準アイコン', sv:'Standardikoner', sr:'Male standardne ikone', si:'සම්මත නිරූපක', eo:'Klasikaj bildsimboloj', sk:'Štandardné ikony', sl:'Standardne ikone', te:'స్టాండర్డ్ ఐకాన్స్', ta:'நிலை படவுறு', th:'ไอคอนมาตรฐาน', tr:'Standart simgeler', cy:'Eiconau safonol', uk:'Стандартні значки', uz:'Стандарт кичик белгилар', es:'Iconos predeterminados', el:'Μικρά πρότυπα εικονίδια', hu:'Szokásos ikonok', it:'Icone standard', hi:'स्टेंडर्ड आइकॉन', id:'Ikon standar', en:'Standard icons', vi:'Biểu tượng tiêu chuẩn', zh_CN:'标准图标'}", INSERT, "id", 48102, "Native-Langue", "Menu", "Main", "Commands");
			action.tagName="Item";
			
			
	
	}
}

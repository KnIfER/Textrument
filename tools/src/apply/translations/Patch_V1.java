package apply.translations;

import static apply.translations.TextrumentLocalePatchs.*;

public class Patch_V1 implements TextrumentLocalePatchs.ActionApplier {
	@Override
	public void pushActions() {
			actions.add(new TextrumentLocalePatchs.Action(null, 0, "Native-Langue", "Dialog", "ShortcutMapper", "MainCommandNames"));

			pushActionByPathIDForNameField("{"+ TextrumentLocalePatchs.LANG.English+":'Current File Path to Clipboard'}", MODIFY, 42029, "Native-Langue", "Menu", "Main", "Commands");
			pushActionByPathIDForNameField("{"+ TextrumentLocalePatchs.LANG.English+":'Current Filename to Clipboard'}", MODIFY, 42030, "Native-Langue", "Menu", "Main", "Commands");
			pushActionByPathIDForNameField("{"+ TextrumentLocalePatchs.LANG.English+":'Current Dir. Path to Clipboard'}", MODIFY, 42031, "Native-Langue", "Menu", "Main", "Commands");

			pushActionByPathIDForNameField("{"+ TextrumentLocalePatchs.LANG.ChineseSimplified+":'清除所有格式'}", MODIFY, 43032, "Native-Langue", "Menu", "Main", "Commands");

			pushActionByPathFieldedIDForNameField("{"+ TextrumentLocalePatchs.LANG.English+":'File Dir Path to Clipboard'}", MODIFY, "CMID", 9, "Native-Langue", "Menu", "TabBar");

			pushActionByPathFieldedIDForNameField("{"+ TextrumentLocalePatchs.LANG.ChineseSimplified+":'复制完整路径到剪切板'}", MODIFY, "CMID", 7, "Native-Langue", "Menu", "TabBar");
			pushActionByPathFieldedIDForNameField("{"+ TextrumentLocalePatchs.LANG.ChineseSimplified+":'复制文件路径到剪切板'}", MODIFY, "CMID", 9, "Native-Langue", "Menu", "TabBar");

			//打开快捷方式所指文件
			pushActionByPathFieldedIDForNameField("{sq:'Të hapur dosjen e lidhur / Directory', ar:'فتح الملف المرتبط / دليل', am:'የተገናኘውን ፋይል ክፈት / ማውጫ', az:'bağlı faylını açın / Directory', ga:'Oscail an comhad nasctha / Eolaire', et:'Avage seotud faili / Directory', eu:'Ireki lotuta fitxategia / direktorioa', be:'Адкрыць звязаны файл / каталог', bg:'Отворете свързаният файл / директория', is:'Opnaðu tengda skrá / möppu', pl:'Otwórz połączony plik / katalog', bs:'Otvorite povezani datoteku / direktorij', fa:'باز کردن فایل مرتبط / راهنمای', af:'Maak die gekoppelde lêer / Directory', da:'Åbn den sammenkædede fil / Bibliotek', de:'Öffnen Sie die verknüpfte Datei / Verzeichnis', ru:'Открыть связанный файл / каталог', fr:'Ouvrez le fichier lié / Directory', tl:'Buksan ang naka-link na file / Directory', fi:'Avaa linkitetty tiedosto / hakemisto', fy:'Iepenje it keppele triem / Directory', km:'បើកឯកសារដែលបានតភ្ជាប់ / ថត', ka:'გახსნა უკავშირდება ფაილი / დირექტორია', gu:'કડી ફાઇલ ખોલો / ડિરેક્ટરી', kk:'байланыстырылған файл / каталогты ашыңыз', ht:'Louvri dosye a lye / Anyè', ko:'링크 된 파일을 엽니 다 / 디렉토리', ha:'Bude da nasaba fayil / Directory', nl:'Open het gekoppelde bestand / Directory', ky:'байланышкан билэни ачуу / Directory', gl:'Abre o arquivo vinculado / Directorio', ca:'Obrir l'arxiu vinculat / Directori', cs:'Otevřete propojený soubor / adresář', kn:'ಡೈರೆಕ್ಟರಿ ತೆರೆಯಿರಿ ಸಂಪರ್ಕ ಫೈಲ್ /', co:'Apre u cartulare ambiguitati / Directory', hr:'Otvorite povezani datoteku / direktorij', ku:'Open the file girêdayî / Directory', la:'Aperi coniunctum lima / Directory', lv:'Atveriet saistītu failu / direktoriju', lo:'ເປີດໄຟລ໌ທີ່ເຊື່ອມໂຍງ / Directory', lt:'Atidaryti susietą failą / katalogą', lb:'Open de verlinkte Fichier / Guide', ro:'Deschideți fișierul legat / Director', mg:'Sokafy ny mifandray rakitra / Directory', mt:'Iftaħ il-fajl marbut / Direttorju', mr:'लिंक फाइल उघडा / निर्देशिका', ml:'ലിങ്കുചെയ്ത ഫയൽ / ഡയറക്ടറി തുറക്കുക', ms:'Buka fail yang berkaitan / Direktori', mk:'Отворете ја оваа датотека / Директориум', mi:'Whakatūwhera te kōnae e hono ana / Directory', mn:'Нээх холбоотой файлыг / сан', bn:'লিঙ্ক ফাইল খুলুন / নির্দেশিকা', my:'ယင်းနှင့်ဆက်စပ်ဖိုင် / Directory ကိုဖွင့်ပါ', hmn:'Qhib lub txuas ntaub ntawv / Directory', xh:'Vula ifayile edityaniswe / Directory', zu:'Vula ifayela exhumene / Directory', ne:'लिङ्क गरिएको फाइल खोल्न / निर्देशिका', no:'Åpne den koblede filen / katalogen', pa:'ਨਾਲ ਜੋੜਿਆ ਫਾਇਲ ਨੂੰ ਖੋਲ੍ਹੋ / ਡਾਇਰੈਕਟਰੀ', pt:'Abra o arquivo vinculado / Diretório', ps:'د تړاو د دوتنې Directory دابرخه دکتابتون /', ny:'Tsegulani file zogwirizana / Directory', ja:'リンクされたファイル/ディレクトリを開きます。', sv:'Öppna den länkade filen / Directory', sm:'Tatala le faila fesootai / Directory', sr:'Отворите повезани фајл / Дирецтори', st:'Bula faele amahanngoa / Directory', si:'සම්බන්ධිත ගොනුව / නාමාවලිය විවෘත', eo:'Malfermu la ligitaj dosiero / Katalogo', sk:'Otvorte prepojený súbor / adresár', sl:'Odprite povezano datoteko / imenik', sw:'Kufungua faili wanaohusishwa / Directory', gd:'Fosgail an ceangal faidhle / Directory', ceb:'Ablihi ang nalambigit file / Directory', so:'Fur faylka lala / Directory', tg:'Кушодани файл алоқаманд / Directory', te:'లింక్ ఫైలు తెరవండి / డైరెక్టరీ', ta:'அடைவு திறந்து இணைக்கப்பட்ட கோப்பு /', th:'เปิดแฟ้มที่เชื่อมโยง / ไดเรกทอรี', tr:'bağlantılı dosyayı açın / Directory', cy:'Agorwch y ffeil cysylltiedig / Directory', ur:'منسلک فائل کو کھولیں / ڈائریکٹری', uk:'Відкрити пов'язаний файл / каталог', uz:'bog'liq faylni oching / Directory', es:'Abrir el archivo vinculado / Directorio', iw:'פתח את הקובץ המקושר / מדריך', el:'Ανοίξτε το συνδεδεμένο αρχείο / κατάλογο', haw:'E wehe i ka ua hoʻopili waihona / Papa Kuhikuhi', sd:'کولڻ جي جڙيل فائيل / فھرست', hu:'Nyissa meg a csatolt fájlt / Directory', sn:'Zarura yakabatana faira / Directory', hy:'Բացեք կապված ֆայլը / ՏԵՂԵԿԱՏՈՒ', ig:'Mepee jikọrọ file / Directory', it:'Aprire il file collegato / Directory', yi:'עפענען די לינגקט טעקע / דירעקטארי', hi:'लिंक की गई फ़ाइल खोलें / निर्देशिका', su:'Buka berkas numbu / Diréktori', id:'Buka file terkait / Directory', jw:'Bukak file disambung / Directory', en:'Open the linked file / Directory', yo:'Ṣii awọn ti sopọ mọ faili / Directory', vi:'Mở tập tin liên kết / Thư mục', zh-TW:'打開鏈接的文件/目錄', zh-CN:'打开快捷方式所指文件', }", INSERT, "CMID", 23, "Native-Langue", "Menu", "TabBar");
	
	
	
			
			
			
	
	}
}

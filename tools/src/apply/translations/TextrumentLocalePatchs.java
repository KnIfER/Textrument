package apply.translations;

import com.alibaba.fastjson.JSONObject;
import org.apache.commons.lang.ArrayUtils;
import org.jdom.Element;
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;
import org.junit.Test;

import java.io.File;
import java.io.FileOutputStream;
import java.io.StringWriter;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;


public class TextrumentLocalePatchs {
	static final int DELETE = 0;
	static final int MODIFY = 1;
	static final int INSERT = 2;
	/** 这是我们感兴趣的子资料文件夹*/
	static String[] filters = new String[]{
			"afrikaans.xml"
			, "albanian.xml"
			, "arabic.xml"
			, "aragonese.xml"
			, "aranese.xml"
			, "azerbaijani.xml"
			, "basque.xml"
			, "belarusian.xml"
			, "bengali.xml"
			, "bosnian.xml"
			, "brazilian_portuguese.xml"
			, "breton.xml"
			, "bulgarian.xml"
			, "catalan.xml"
			, "chineseSimplified.xml"
			, "chineseTraditional.xml"
			, "corsican.xml"
			, "croatian.xml"
			, "czech.xml"
			, "danish.xml"
			, "dutch.xml"
			, "english.xml"
			, "english_customizable.xml"
			, "esperanto.xml"
			, "estonian.xml"
			, "extremaduran.xml"
			, "farsi.xml"
			, "finnish.xml"
			, "french.xml"
			, "friulian.xml"
			, "galician.xml"
			, "georgian.xml"
			, "german.xml"
			, "greek.xml"
			, "gujarati.xml"
			, "hebrew.xml"
			, "hindi.xml"
			, "hungarian.xml"
			, "indonesian.xml"
			, "irish.xml"
			, "italian.xml"
			, "japanese.xml"
			, "kabyle.xml"
			, "kannada.xml"
			, "kazakh.xml"
			, "korean.xml"
			, "kurdish.xml"
			, "kyrgyz.xml"
			, "latvian.xml"
			, "ligurian.xml"
			, "lithuanian.xml"
			, "luxembourgish.xml"
			, "macedonian.xml"
			, "malay.xml"
			, "marathi.xml"
			, "mongolian.xml"
			, "nepali.xml"
			, "norwegian.xml"
			, "nynorsk.xml"
			, "occitan.xml"
			, "piglatin.xml"
			, "polish.xml"
			, "portuguese.xml"
			, "punjabi.xml"
			, "romanian.xml"
			, "russian.xml"
			, "samogitian.xml"
			, "sardinian.xml"
			, "serbian.xml"
			, "serbianCyrillic.xml"
			, "sinhala.xml"
			, "slovak.xml"
			, "slovenian.xml"
			, "spanish.xml"
			, "spanish_ar.xml"
			, "swedish.xml"
			, "tagalog.xml"
			, "taiwanese.xml"
			, "tajikCyrillic.xml"
			, "tamil.xml"
			, "tatar.xml"
			, "telugu.xml"
			, "thai.xml"
			, "turkish.xml"
			, "ukrainian.xml"
			, "urdu.xml"
			, "uyghur.xml"
			, "uzbek.xml"
			, "uzbekCyrillic.xml"
			, "venetian.xml"
			, "vietnamese.xml"
			, "welsh.xml"
			, "zulu.xml"
	};
	
	static LANG[] shortNames = new LANG[] {
			LANG.Afrikaans
			, LANG.Albanian
			, LANG.Arabic
			, null // "aragonese.xml"
			, null // "aranese.xml"
			, LANG.Azerbaijani
			, LANG.Basque
			, LANG.Belarusian
			, LANG.Bengali
			, LANG.Bosnian
			, LANG.Portuguese //"brazilian_portuguese.xml"
			, null // "breton.xml"
			, LANG.Bulgarian
			, LANG.Catalan
			, LANG.ChineseSimplified
			, LANG.ChineseTraditional
			, LANG.Corsican
			, LANG.Croatian
			, LANG.Czech
			, LANG.Danish
			, LANG.Dutch
			, LANG.English
			, LANG.English
			, LANG.Esperanto
			, LANG.Estonian
			, null // "extremaduran.xml"
			, null // "farsi.xml"
			, LANG.Finnish
			, LANG.French
			, null // "friulian.xml"
			, LANG.Galician
			, LANG.Georgian
			, LANG.German
			, LANG.Greek
			, LANG.Gujarati
			, LANG.Hebrew
			, LANG.Hindi
			, LANG.Hungarian
			, LANG.Indonesian
			, LANG.Irish
			, LANG.Italian
			, LANG.Japanese
			, null // "kabyle.xml"
			, LANG.Kannada
			, LANG.Kazakh
			, LANG.Korean
			, LANG.Kurdish
			, LANG.Kyrgyz
			, LANG.Latvian
			, null // "ligurian.xml"
			, LANG.Lithuanian
			, LANG.Luxembourgish
			, LANG.Macedonian
			, LANG.Malay
			, LANG.Marathi
			, LANG.Mongolian
			, LANG.Nepali
			, LANG.Norwegian
			, null // "nynorsk.xml"
			, null // "occitan.xml"
			, null // "piglatin.xml"
			, LANG.Polish
			, LANG.Portuguese
			, LANG.Punjabi
			, LANG.Romanian
			, LANG.Russian
			, null // "samogitian.xml"
			, null // "sardinian.xml"
			, LANG.Serbian
			, null // "serbianCyrillic.xml"
			, LANG.Sinhala
			, LANG.Slovak
			, LANG.Slovenian
			, LANG.Spanish
			, null // "spanish_ar.xml"
			, LANG.Swedish
			, null // "tagalog.xml"
			, null // "taiwanese.xml"
			, null // "tajikCyrillic.xml"
			, LANG.Tamil
			, null // "tatar.xml"
			, LANG.Telugu
			, LANG.Thai
			, LANG.Turkish
			, LANG.Ukrainian
			, LANG.Urdu
			, null // "uyghur.xml"
			, LANG.Uzbek
			, LANG.Uzbek //"uzbekCyrillic.xml"
			, null // "venetian.xml"
			, LANG.Vietnamese
			, LANG.Welsh
			, LANG.Zulu
	};

	static HashMap<String, Integer> shortName_id_table = new HashMap<>();

	static {
		for (int i = 0; i < shortNames.length; i++) {
			if(shortNames[i]!=null) {
				shortName_id_table.put(shortNames[i].code, i);
			}
		}
	}

	static String[] androidCps = new String[]{
			"afrikaans.xml"
			, "albanian.xml"
			, "values-ar.xml"
			, "aragonese.xml"
			, "aranese.xml"
			, "azerbaijani.xml"
			, "basque.xml"
			, "belarusian.xml"
			, "bengali.xml"
			, "bosnian.xml"
			, "brazilian_portuguese.xml"
			, "breton.xml"
			, "bulgarian.xml"
			, "catalan.xml"
			, "values-zh-rCN"
			, "values-zh-rHK"
			, "corsican.xml"
			, "croatian.xml"
			, "czech.xml"
			, "danish.xml"
			, "dutch.xml"
			, "english.xml"
			, "english_customizable.xml"
			, "esperanto.xml"
			, "estonian.xml"
			, "extremaduran.xml"
			, "farsi.xml"
			, "finnish.xml"
			, "french.xml"
			, "friulian.xml"
			, "galician.xml"
			, "georgian.xml"
			, "german.xml"
			, "greek.xml"
			, "gujarati.xml"
			, "hebrew.xml"
			, "hindi.xml"
			, "hungarian.xml"
			, "indonesian.xml"
			, "irish.xml"
			, "italian.xml"
			, "values-ja"
			, "kabyle.xml"
			, "kannada.xml"
			, "kazakh.xml"
			, "values-ko-rKR"
			, "kurdish.xml"
			, "kyrgyz.xml"
			, "latvian.xml"
			, "ligurian.xml"
			, "lithuanian.xml"
			, "luxembourgish.xml"
			, "macedonian.xml"
			, "malay.xml"
			, "marathi.xml"
			, "mongolian.xml"
			, "nepali.xml"
			, "norwegian.xml"
			, "nynorsk.xml"
			, "occitan.xml"
			, "piglatin.xml"
			, "polish.xml"
			, "portuguese.xml"
			, "punjabi.xml"
			, "romanian.xml"
			, "russian.xml"
			, "samogitian.xml"
			, "sardinian.xml"
			, "serbian.xml"
			, "serbianCyrillic.xml"
			, "sinhala.xml"
			, "slovak.xml"
			, "slovenian.xml"
			, "spanish.xml"
			, "spanish_ar.xml"
			, "swedish.xml"
			, "tagalog.xml"
			, null
			, "tajikCyrillic.xml"
			, "tamil.xml"
			, "tatar.xml"
			, "telugu.xml"
			, "thai.xml"
			, "turkish.xml"
			, "ukrainian.xml"
			, "urdu.xml"
			, "values-ug-rCN.xml"
			, "uzbek.xml"
			, "uzbekCyrillic.xml"
			, "venetian.xml"
			, "values-vi-rVN"
			, "welsh.xml"
			, "zulu.xml"
	};

	/** 这是我们的源文件夹*/
	static File sourceFolder=new File("D:\\Code\\FigureOut\\notepad-plus-plus\\PowerEditor\\installer\\nativeLang");

	/** Simply transfer key-values pair among xmls. */
    public static void main(String[] args) throws Exception {
		new File(sourceFolder, "taiwaneseMandarin.xml").renameTo(new File(sourceFolder, "taiwanese.xml"));

		
    }


    static class Action {
    	/** 0=delete; 1=modify; 2=insert */
    	int type;
    	String[] XMLPath;
    	String[] values;// = new String[filters.length];
		String idField="id";
		String id=null;
		String fieldName=null;
		Action(String values, int type, String...XMLPath) {
			this.type = type;
			this.XMLPath = XMLPath;
			if(values!=null) {
				JSONObject jsonObj = JSONObject.parseObject(values);
				String[] keyArr = jsonObj.keySet().toArray(ArrayUtils.EMPTY_STRING_ARRAY);
				this.values = new String[filters.length];
				for(String kI:keyArr) {
					Integer id = shortName_id_table.get(kI.trim());
					if(id!=null) {
						this.values[id] = jsonObj.getString(kI);
					}
				}
			}
		}
	}

	static ArrayList<Action> actions = new ArrayList<>(128);


	static void pushActionByPathIDForNameField(String jsonTrans, int type, int id, String...XMLPath) {
		Action action = new Action(jsonTrans, type, XMLPath);
		action.id = Integer.toString(id);
		action.fieldName = "name";
		actions.add(action);
	}

	static void pushActionByPathFieldedIDForNameField(String jsonTrans, int type, String idField, int id, String...XMLPath) {
		Action action = new Action(jsonTrans, type, XMLPath);
		action.idField = idField;
		action.id = Integer.toString(id);
		action.fieldName = "name";
		actions.add(action);
	}

	
	@Test
	public void TestJDOM() throws Exception {
		new Patch_V1().pushActions();
		processXmlFileById(shortName_id_table.get(LANG.ChineseSimplified.code), false);
	}
	
	public interface ActionApplier {
		void pushActions();
	}

	/** Process one xml file by id. */
	public void processXmlFileById(int id, boolean test) throws Exception {
		
		String xmlFileName = filters[id];

		File testFile = new File(sourceFolder, xmlFileName);

		String localPath = new File("").getAbsolutePath();
		SAXBuilder builder = new SAXBuilder();
		org.jdom.Document document = builder.build(testFile);

		for (int i = 0; i < actions.size(); i++) {
			Action aI = actions.get(i);
			if(aI.type==0) {
				Element _toDel = getElementByPath(document.getRootElement(), aI.XMLPath);
				if(_toDel!=null) {
					Element toDelp = (Element) _toDel.getParent();
					toDelp.removeChild(_toDel.getName());
				}
			} else if(aI.type==1) {
				String value = aI.values[id];
				if(value!=null) {
					Element _toMod = getElementByPath(document.getRootElement(), aI.XMLPath);
					if(_toMod!=null) {
						if(aI.id!=null) {
							List<Element> childs = _toMod.getChildren();
							_toMod = null;
							for(Element cI:childs) {
								if(aI.id.equals(cI.getAttributeValue(aI.idField))) {
									_toMod = cI;
									break;
								}
							}
						}
					}
					if(_toMod!=null) {
						if(aI.fieldName!=null) {
							_toMod.setAttribute(aI.fieldName, value);
						} else {
							_toMod.setText(value);
						}
					}
				}
			}

		}

		XMLOutputter xmlOutput = new XMLOutputter();
		Format f = Format.getRawFormat();
		//f.setIndent("  "); // 文本缩进
		//f.setTextMode(Format.TextMode.TRIM_FULL_WHITE);
		xmlOutput.setFormat(f);
		//xmlOutput.output(document, new FileOutputStream(testFile));
		StringWriter sw = new StringWriter();
		xmlOutput.output(document, sw);

		String text = sw.toString();

		text = text.replace(" />", "/>");

		if(test) {
			Log(text);
		} else {
			FileOutputStream fo = new FileOutputStream(testFile);
			fo.write(text.getBytes(StandardCharsets.UTF_8));
			fo.flush();
			fo.close();
		}
	}
	
	private Element getElementByPath(Element rootElement, String...names) {
		for(String nI:names) {
			rootElement = rootElement.getChild(nI);
			if(rootElement==null) {
				return null;
			}
		}
    	return rootElement;
	}

	public enum LANG {
		Auto("auto", "Auto", "Auto"),
		Albanian("sq", "Shqiptar", "Albanian"),
		Arabic("ar", "العربية", "Arabic"),
		Amharic("am", "አማርኛ", "Amharic"),
		Azerbaijani("az", "Azərbaycan", "Azerbaijani"),
		Irish("ga", "Irish", "Irish"),
		Estonian("et", "Eesti keel", "Estonian"),
		Basque("eu", "Euskal", "Basque"),
		Belarusian("be", "беларускі", "Belarusian"),
		Bulgarian("bg", "Български", "Bulgarian"),
		Icelandic("is", "Íslensku", "Icelandic"),
		Polish("pl", "Polski", "Polish"),
		Bosnian("bs", "Bosanski", "Bosnian"),
		Persian("fa", "فارسی", "Persian"),
		Afrikaans("af", "Afrikaans", "Afrikaans"),
		Danish("da", "Dansk", "Danish"),
		German("de", "Deutsch", "German"),
		Russian("ru", "Русский", "Russian"),
		French("fr", "Français", "French"),
		Filipino("tl", "Pilipino", "Filipino"),
		Finnish("fi", "Suomalainen", "Finnish"),
		Frisian("fy", "Frysk", "Frisian"),
		Khmer("km", "ភាសាខ្មែរ", "Khmer"),
		Georgian("ka", "ქართული", "Georgian"),
		Gujarati("gu", "ગુજરાતી", "Gujarati"),
		Kazakh("kk", "Қазақша", "Kazakh"),
		HaitianCreole("ht", "Kreyòl Ayisyen", "Haitian Creole"),
		Korean("ko", "한국어", "Korean"),
		Hausa("ha", "Hausa", "Hausa"),
		Dutch("nl", "Nederlands", "Dutch"),
		Kyrgyz("ky", "Кыргыз тили", "Kyrgyz"),
		Galician("gl", "Galego", "Galician"),
		Catalan("ca", "Català", "Catalan"),
		Czech("cs", "Česky", "Czech"),
		Kannada("kn", "ಕನ್ನಡ", "Kannada"),
		Corsican("co", "Corsa", "Corsican"),
		Croatian("hr", "hrvatski", "Croatian"),
		Kurdish("ku", "Kurdî", "Kurdish"),
		Latin("la", "Latina", "Latin"),
		Latvian("lv", "Latviešu", "Latvian"),
		Laotian("lo", "Laotian", "Laotian"),
		Lithuanian("lt", "Lietuviškai", "Lithuanian"),
		Luxembourgish("lb", "Lëtzebuergesch", "Luxembourgish"),
		Romanian("ro", "Românesc", "Romanian"),
		Malagasy("mg", "Malagasy", "Malagasy"),
		Maltese("mt", "Malti", "Maltese"),
		Marathi("mr", "मराठी", "Marathi"),
		Malayalam("ml", "മലയാളം", "Malayalam"),
		Malay("ms", "Melayu", "Malay"),
		Macedonian("mk", "Македонски", "Macedonian"),
		Maori("mi", "Maori", "Maori"),
		Mongolian("mn", "Монгол хэл", "Mongolian"),
		Bengali("bn", "বাংলা ভাষার", "Bengali"),
		Burmese("my", "မြန်မာ", "Burmese"),
		Hmong("hmn", "Hmoob", "Hmong"),
		Xhosa("xh", "IsiXhosa", "Xhosa"),
		Zulu("zu", "Zulu", "Zulu"),
		Nepali("ne", "नेपाली", "Nepali"),
		Norwegian("no", "Norsk språk", "Norwegian"),
		Punjabi("pa", "ਪੰਜਾਬੀ", "Punjabi"),
		Portuguese("pt", "Português", "Portuguese"),
		Pashto("ps", "پښتو", "Pashto"),
		Chichewa("ny", "Chichewa", "Chichewa"),
		Japanese("ja", "日本語", "Japanese"),
		Swedish("sv", "Svenska", "Swedish"),
		Samoan("sm", "Samoa", "Samoan"),
		Serbian("sr", "Српски", "Serbian"),
		Sotho("st", "Sesotho", "Sotho"),
		Sinhala("si", "සිංහල", "Sinhala"),
		Esperanto("eo", "Esperanta", "Esperanto"),
		Slovak("sk", "Slovenčina", "Slovak"),
		Slovenian("sl", "Slovenščina", "Slovenian"),
		SwahiliSwahili("sw", "Kiswahili", "Swahili"),
		ScottishGaelic("gd", "Gàidhlig na h-Alba", "Scottish Gaelic"),
		Cebuano("ceb", "Cebuano", "Cebuano"),
		Somali("so", "Somali", "Somali"),
		Tajik("tg", "Тоҷикӣ", "Tajik"),
		Telugu("te", "తెలుగు", "Telugu"),
		Tamil("ta", "தமிழ்", "Tamil"),
		Thai("th", "ไทย", "Thai"),
		Turkish("tr", "Türk", "Turkish"),
		Welsh("cy", "Cymraeg", "Welsh"),
		Urdu("ur", "اردو", "Urdu"),
		Ukrainian("uk", "Український", "Ukrainian"),
		Uzbek("uz", "O'zbek", "Uzbek"),
		Spanish("es", "Español", "Spanish"),
		Hebrew("iw", "עברית", "Hebrew"),
		Greek("el", "Ελληνικά", "Greek"),
		Hawaiian("haw", "Hawaiian", "Hawaiian"),
		Sindhi("sd", "سنڌي", "Sindhi"),
		Hungarian("hu", "Magyar", "Hungarian"),
		Shona("sn", "Shona", "Shona"),
		Armenian("hy", "Հայերեն", "Armenian"),
		Igbo("ig", "Igbo", "Igbo"),
		Italian("it", "Italiano", "Italian"),
		Yiddish("yi", "ייִדיש", "Yiddish"),
		Hindi("hi", "हिंदी", "Hindi"),
		Sundanese("su", "Sunda", "Sundanese"),
		Indonesian("id", "Indonesia", "Indonesian"),
		Javanese("jw", "Wong Jawa", "Javanese"),
		English("en", "English", "English"),
		Yoruba("yo", "Yorùbá", "Yoruba"),
		Vietnamese("vi", "Tiếng Việt", "Vietnamese"),
		ChineseTraditional("zh-TW", "正體中文", "Chinese Traditional"),
		ChineseSimplified("zh-CN", "简体中文", "Chinese Simplified");

		private String code;
		private String name;
		private String englishName;

		LANG(String code, String name, String englishName) {
			this.code = code;
			this.name = name;
			this.englishName = englishName;
		}

		public String getCode() {
			return code;
		}

		public void setCode(String code) {
			this.code = code;
		}

		public String getName() {
			return name;
		}

		public void setName(String name) {
			this.name = name;
		}

		public String getEnglishName() {
			return englishName;
		}

		public void setEnglishName(String englishName) {
			this.englishName = englishName;
		}

		@Override
		public String toString() {
			return code;
		}

	}
	
	private static void Log(String name) {
		System.out.println(name);
	}
}



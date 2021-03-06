package apply.translations;

import com.alibaba.fastjson.JSONObject;
import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.jdom.Element;
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;
import org.junit.Test;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import static apply.translations.Install.installXmlNativeByLocale;


public class TextrumentLocalePatchs {
	static final int DELETE = 0;
	static final int MODIFY = 1;
	static final int INSERT = 2;
	static final int EXTRACT = 3;
	static final int MERGE = 4;
	static final int MOVE = 5;
	static final int RENAME = 6;
	static final int REMOVECONTENTS = 7;
	static String actionGetName(int action) {
		switch (action) {
			case DELETE:
			return "DELETE";
			case MODIFY:
			return "MODIFY";
			case INSERT:
			return "INSERT";
		}
		return "EXTRACT";
	}
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
			, "hongKongCantonese.xml"
			, "abkhazian.xml"
	};
	
	static LANG[] shortNames = new LANG[] {
			LANG.Afrikaans
			, LANG.Albanian
			, LANG.Arabic
			, LANG.aragonese // "aragonese.xml"
			, LANG.aranese//LANG.aranese // "aranese.xml"
			, LANG.Azerbaijani
			, LANG.Basque
			, LANG.Belarusian
			, LANG.Bengali
			, LANG.Bosnian
			, LANG.Portuguese_b //"brazilian_portuguese.xml"
			, LANG.breton // "breton.xml"
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
			, LANG.English1
			, LANG.Esperanto
			, LANG.Estonian
			, LANG.extremaduran // "extremaduran.xml"
			, LANG.farsi // "farsi.xml"
			, LANG.Finnish
			, LANG.French
			, LANG.friulian // "friulian.xml"
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
			, LANG.kabyle // "kabyle.xml"
			, LANG.Kannada
			, LANG.Kazakh
			, LANG.Korean
			, LANG.Kurdish
			, LANG.Kyrgyz
			, LANG.Latvian
			, LANG.ligurian // "ligurian.xml"
			, LANG.Lithuanian
			, LANG.Luxembourgish
			, LANG.Macedonian
			, LANG.Malay
			, LANG.Marathi
			, LANG.Mongolian
			, LANG.Nepali
			, LANG.Norwegian
			, LANG.nynorsk // "nynorsk.xml"
			, LANG.occitan // "occitan.xml"
			, LANG.piglatin // "piglatin.xml"
			, LANG.Polish
			, LANG.Portuguese
			, LANG.Punjabi
			, LANG.Romanian
			, LANG.Russian
			, LANG.Samoan // "samogitian.xml"
			, LANG.sardinian // "sardinian.xml"
			, LANG.Serbian
			, LANG.serbianCyrillic // "serbianCyrillic.xml"
			, LANG.Sinhala
			, LANG.Slovak
			, LANG.Slovenian
			, LANG.Spanish
			, LANG.spanish_ar // "spanish_ar.xml"
			, LANG.Swedish
			, LANG.tagalog // "tagalog.xml"
			, LANG.taiwanese // "taiwanese.xml"
			, LANG.Tajik // "tajikCyrillic.xml"
			, LANG.Tamil
			, LANG.tatar // "tatar.xml"
			, LANG.Telugu
			, LANG.Thai
			, LANG.Turkish
			, LANG.Ukrainian
			, LANG.Urdu
			, LANG.uyghur // "uyghur.xml"
			, LANG.Uzbek
			, LANG.Uzbek_b //"uzbekCyrillic.xml"
			, LANG.venetian // "venetian.xml"
			, LANG.Vietnamese
			, LANG.Welsh
			, LANG.Zulu
			, LANG.hongKongCantonese
			, LANG.abkhazian
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
			, null
			, null
	};

	/** 这是我们的源文件夹*/
	public static File sourceFolder=new File("..\\PowerEditor\\installer\\nativeLang");

	/** Simply transfer key-values pair among xmls. */
    public static void main(String[] args) throws Exception {
		File f = new File(sourceFolder, "taiwaneseMandarin.xml");
		File t = new File(sourceFolder, "taiwanese.xml");
		if (f.exists()&&t.exists()) {
			t.delete();
		}
		f.renameTo(t);
		
		for(LANG idx:LANG.values()) {
			processXmlFileByEnum(idx, false);
		}
    }
    
	@Test
	public void TestJDOM() throws Exception {
		processXmlFileByEnum(LANG.ChineseSimplified, false);
		installXmlNativeByLocale(LANG.ChineseSimplified);
	}
	
	@Test
	public void TestPatches() throws Exception {
		processXmlFileByEnum(LANG.English, false);
		processXmlFileByEnum(LANG.ChineseSimplified, false);
	}

    static class Action {
		/** 0=delete; 1=modify; 2=insert */
    	int type;
		boolean mkdirs=false;
    	String[] XMLPath;
    	String[] XMLPath1;
    	String[] values;// = new String[filters.length];
		String tagName;
		String idField="id";
		String id=null;
		String fieldName=null;
		StringConverter converter=null;
		Action(String values, int type, String...XMLPath) {
			this.type = type;
			this.XMLPath = XMLPath;
			if(values!=null) {
				JSONObject jsonObj = JSONObject.parseObject(values);
				String[] keyArr = jsonObj.keySet().toArray(ArrayUtils.EMPTY_STRING_ARRAY);
				this.values = new String[filters.length];
				for(String kI:keyArr) {// 分发
					Integer id = shortName_id_table.get(kI.trim());
					if(id!=null) {
						this.values[id] = jsonObj.getString(kI);
					}
				}
				this.values[shortName_id_table.get(LANG.English1.code)]=this.values[shortName_id_table.get(LANG.English.code)];
			}
		}
	}
	
	public interface ActionApplier {
		void pushActions();
	}
	
	public interface StringConverter {
		String editInput(String input);
	}
	
	static ArrayList<Action> actions = new ArrayList<>(128);

	static {
		Patches.pushAll();
	}
	
	static void pushActionByPathIDForNameField(String jsonTrans, int type, int id, String...XMLPath) {
		Action action = new Action(jsonTrans, type, XMLPath);
		action.id = Integer.toString(id);
		action.fieldName = "name";
		actions.add(action);
	}

	static Action pushActionByPathFieldedIDForNameField(String jsonTrans, int type, String idField, Object id, String...XMLPath) {
		Action action = new Action(jsonTrans, type, XMLPath);
		action.idField = idField;
		action.id = id==null?null:""+id;
		action.fieldName = "name";
		actions.add(action);
		return action;
	}
	
	static Action pushActionByPathFieldedIDForFieldInTag(String jsonTrans, int type, String idField, Object id, String nmF, String tagName, String...XMLPath) {
		Action action = new Action(jsonTrans, type, XMLPath);
		action.idField = idField;
		action.id = id==null?null:""+id;
		action.fieldName = nmF;
		action.tagName = tagName;
		actions.add(action);
		return action;
	}
	

	/** Process one xml file by id. */
	public static void processXmlFileByEnum(LANG Enum, boolean test) throws Exception {
		if(shortName_id_table.get(Enum.code)==null) {
			return;
		}
		int id = shortName_id_table.get(Enum.code);
		
		String xmlFileName = filters[id];

		File file = new File(sourceFolder, xmlFileName);
		
		//Log(file.getPath());
		
		if(!file.exists()) {
			return;
		}

		byte[] data = new byte[(int) file.length()];
		FileInputStream fin = new FileInputStream(file);
		fin.read(data);
		fin.close();
		String xmlText = new String(data, StandardCharsets.UTF_8);

		StringBuffer sb = new StringBuffer(xmlText.length());
		
		ArrayList<String> arrComments = new ArrayList<>();
		Pattern commentPattern = Pattern.compile("(<!--.*?-->)", Pattern.DOTALL);
		Matcher commentMatcher = commentPattern.matcher(xmlText);
		while (commentMatcher.find()) {
			arrComments.add(commentMatcher.group(1));
			commentMatcher.appendReplacement(sb, "<COMMENT_"+(arrComments.size()-1)+"/>");
		}
		commentMatcher.appendTail(sb);
		xmlText = sb.toString();
		
		
		Pattern p = Pattern.compile("(\\r\\n)+\\S(?<!<)");
		Matcher m = p.matcher(xmlText);
		sb.setLength(0);
		while(m.find()) {
			//Log(m.groupCount()+""); Log(m.group());
			// https://blog.csdn.net/qq_37502106/article/details/88642840
			// for temporary replacements of wrapped comments( <!--***\n ).
			m.appendReplacement(sb, java.util.regex.Matcher.quoteReplacement(m.group(0).replace("\r\n", "|LFCR|")));
		}
		m.appendTail(sb);
		xmlText = sb.toString();
		
		
		xmlText = xmlText.replace("\r\n\r\n", "<!--LFCR-->");

		commentPattern = Pattern.compile("<COMMENT_([0-9]+)/>");
		commentMatcher = commentPattern.matcher(xmlText);
		sb.setLength(0);
		while (commentMatcher.find()) {
			int idx = Integer.parseInt(commentMatcher.group(1));
			commentMatcher.appendReplacement(sb, "");
			sb.append(arrComments.get(idx));
		}
		commentMatcher.appendTail(sb);
		xmlText = sb.toString();
		
		org.jdom.Document document = null;

		try {
			document = new SAXBuilder().build(new StringReader(xmlText));
		} catch (Exception e) {
			Log(file.getPath());
			Log(xmlText);
			e.printStackTrace();
			System.exit(0);
			return;
		}

		for (int i = 0; i < actions.size(); i++) {
			Action aI = actions.get(i);
			switch (aI.type) {
				case DELETE: {
					if (aI.values==null||aI.values[id]!=null) {
						// deletion filters
						Element _toDel = getElementByPath(document.getRootElement(), aI.XMLPath);
						if (_toDel != null) {
							if (aI.id != null) {
								List<Element> childs = _toDel.getChildren();
								_toDel = null;
								for (Element cI : childs) {
									if (aI.id.equals(cI.getAttributeValue(aI.idField))) {
										_toDel = cI;
										break;
									}
								}
							}
						}
						if (_toDel != null) {
							Element toDelp = (Element) _toDel.getParent();
							toDelp.removeContent(_toDel);
						}
					}
				} break;
				case MODIFY: {
					String value=null;
					if(aI.converter!=null) {
						value= StringUtils.EMPTY;
					} else {
						value = aI.values[id];
					}
					if (value != null) {
						Element _toMod = getElementByPath(document.getRootElement(), aI.XMLPath);
						if (_toMod != null) {
							if (aI.id != null) {
								List<Element> childs = _toMod.getChildren();
								_toMod = null;
								for (Element cI : childs) {
									if (aI.id.equals(cI.getAttributeValue(aI.idField))) {
										_toMod = cI;
										break;
									}
								}
							}
						}
						if (_toMod != null) {
							if(aI.converter!=null) {
								value=_toMod.getAttributeValue(aI.fieldName);
								value = aI.converter.editInput(value);
								if(value==null) {
									continue;
								}
							}
							if (aI.fieldName != null) {
								_toMod.setAttribute(aI.fieldName, value);
							} else {
								_toMod.setText(value);
							}
							break;
						} else if(aI.values==null) {
							break;
						}
						// else fallback to insert
					} else {
						break;
					}
				}
				case INSERT: {
					String value = aI.values[id];
					if (value != null && aI.tagName != null) {
						Element _toInsp = getElementByPath(document.getRootElement(), aI.mkdirs, aI.XMLPath);
						if (_toInsp != null) {
							Element ele = new Element(aI.tagName);
							if (aI.id != null) {
								if(hasChildElementWithId(_toInsp.getChildren(), aI.idField, aI.id)) {
									continue;
								}
								ele.setAttribute(aI.idField, aI.id);
							}
							if (aI.fieldName != null) {
								ele.setAttribute(aI.fieldName, value);
							} else {
								ele.setText(value);
							}

							_toInsp.addContent(ele);
						}
					}
					break;
				}
				case EXTRACT: {
					Element _toExt = getElementByPath(document.getRootElement(), aI.XMLPath);
					String value=null;
					if (_toExt != null) {
						if (aI.id != null) {
							_toExt = getChildElementWithId(_toExt.getChildren(), aI.idField, aI.id);
						}
					}
					if(_toExt!=null) {
						if(aI.fieldName!=null) {
							value = _toExt.getAttributeValue(aI.fieldName);
						} else {
							value = _toExt.getValue();
						}
					}
					if(value!=null) {
						buf.append(Enum.code);
						buf.append(":'");
						buf.append(value.replaceAll("'", "\\\\\\\\'"));
						buf.append("', ");
					}
					break;
				}
				case MERGE: {
					Element _toExt = getElementByPath(document.getRootElement(), aI.XMLPath);
					Element _fromExt = getElementByPath(document.getRootElement(), aI.XMLPath1);
					if (_toExt != null && _fromExt!=null) {
						List<Element> childs = new ArrayList<>(_fromExt.getChildren());
						for (Element child : childs) {
							_fromExt.removeContent(child);
							_toExt.addContent(child);
						}
						_fromExt.getParent().removeContent(_fromExt);
					}
					break;
				}
				case MOVE: {
					Element _toExt = getElementByPath(document.getRootElement(), aI.XMLPath);
					Element _fromExt = getElementByPath(document.getRootElement(), aI.XMLPath1);
					if (_toExt != null) {
						Element _toMod = _fromExt;
						if (_toMod != null) {
							if (aI.id != null) {
								List<Element> childs = _toMod.getChildren();
								_toMod = null;
								for (Element cI : childs) {
									if (aI.id.equals(cI.getAttributeValue(aI.idField))) {
										_toMod = cI;
										break;
									}
								}
							}
						}
						//Log("moving... "+Arrays.asList(aI.XMLPath)+" -> "+Arrays.asList(aI.XMLPath1)+" "+aI.idField);
						if (_toMod != null) {
							_toMod.getParent().removeContent(_toMod);
							_toExt.addContent(_toMod);
						}
					}
					break;
				}
				case RENAME: {
					String value= aI.tagName;
					if (value != null) {
						Element _toMod;
						if (aI.XMLPath==null) {
							_toMod = document.getRootElement();
						}
						else {
							_toMod = getElementByPath(document.getRootElement(), aI.XMLPath);
						}
						if (_toMod != null) {
							if (aI.id != null) {
								List<Element> childs = _toMod.getChildren();
								_toMod = null;
								for (Element cI : childs) {
									if (aI.id.equals(cI.getAttributeValue(aI.idField))) {
										_toMod = cI;
										break;
									}
								}
							}
						}
						if (_toMod != null) {
							_toMod.setName(value);
						}
					}
					break;
				}
				case REMOVECONTENTS: {
					String value= aI.tagName;
					if (value != null) {
						Element _toMod = getElementByPath(document.getRootElement(), aI.XMLPath);
						if (_toMod != null) {
							if (aI.id != null) {
								List<Element> childs = _toMod.getChildren();
								_toMod = null;
								for (Element cI : childs) {
									if (aI.id.equals(cI.getAttributeValue(aI.idField))) {
										_toMod = cI;
										break;
									}
								}
							}
						}
						if (_toMod != null) {
							for (int j = _toMod.getContentSize()-1; j >= 0; j--) {
								_toMod.removeContent(j);
							}
						}
					}
					break;
				}
			}

		}
		
		
		XMLOutputter xmlOutput = new XMLOutputter();
		Format f = Format.getRawFormat();
		f.setIndent("\t"); // 文本缩进
		f.setLineSeparator("\r\n");
		f.setTextMode(Format.TextMode.PRESERVE);
		xmlOutput.setFormat(f);
		//xmlOutput.output(document, new FileOutputStream(testFile));
		StringWriter sw = new StringWriter();
		xmlOutput.output(document, sw);

		xmlText = sw.toString();

		xmlText = xmlText.replace(" />", "/>");

		xmlText = xmlText.replaceAll("\\s+\\r\\n", "\r\n");

		xmlText = xmlText.replace("|LFCR|", "\r\n");
		xmlText = xmlText.replaceAll(".*<!--LFCR-->\r\n", "\r\n");

		xmlText = xmlText.replace("Notepad++", "Textrument");
		
		
		
		if(test) {
			if(actions.get(0).type!=EXTRACT)Log(xmlText);
		} else {
			FileOutputStream fo = new FileOutputStream(file);
			fo.write(xmlText.getBytes(StandardCharsets.UTF_8));
			fo.flush();
			fo.close();
		}
	}

	private static boolean hasChildElementWithId(List<Element> children, String idField, String id) {
		for (Element child : children) {
			if (id.equals(child.getAttributeValue(idField))) {
				return true;
			}
		}
		return false;
	}
	
	private static Element getChildElementWithId(List<Element> children, String idField, String id) {
		for (Element child : children) {
			if (idField==null) {
				if (child.getName().equals(id)) {
					return child;
				}
			}
			else if (id.equals(child.getAttributeValue(idField))) {
				return child;
			}
		}
		return null;
	}

	private static Element getElementByPath(Element rootElement, String...names) {
    	return getElementByPath(rootElement, false, names);
	}
	
	private static Element getElementByPath(Element rootElement, boolean create, String...names) {
		if(rootElement!=null) {
			Element nextElement;
			for(String nI:names) {
				nextElement = rootElement.getChild(nI);
				if(nextElement==null) {
					if(create) {
						nextElement = new Element(nI);
						rootElement.addContent(nextElement);
					} else {
						return null;
					}
				}
				rootElement = nextElement;
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
		English1("en1", "English", "English"),
		Yoruba("yo", "Yorùbá", "Yoruba"),
		Vietnamese("vi", "Tiếng Việt", "Vietnamese"),
		ChineseTraditional("zh_TW", "正體中文", "Chinese Traditional"),
		ChineseSimplified("zh_CN", "简体中文", "Chinese Simplified")
		
		//todo: Translator below locales in baidu
		, aragonese("arg", "?", "?")
		, breton("bre", "?", "?")
		, friulian("fri", "?", "?")
		, kabyle("kab", "?", "?")
		, nynorsk("nno", "?", "?")
		, occitan("oci", "?", "?")
		, sardinian("sar", "?", "?")
		, venetian("venetian", "?", "?")
		, aranese("aranese", "?", "?")
		, extremaduran("extremaduran", "?", "?")
		, farsi("farsi", "?", "?")
		, ligurian("ligurian", "?", "?")
		, piglatin("piglatin", "?", "?")
		, serbianCyrillic("serbianCyrillic", "?", "?")
		, spanish_ar("spanish_ar", "?", "?")
		, tagalog("tagalog", "?", "?")
		, taiwanese("taiwanese", "?", "?")
		, tatar("tatar", "?", "?")
		, uyghur("uyghur", "?", "?")
		, hongKongCantonese("hongKongCantonese", "?", "?")
		, abkhazian("abkhazian", "?", "?")
		, Uzbek_b("uz_b", "O'zbek", "Uzbek")
		, Portuguese_b("pt_b", "Português", "Portuguese")

				;
		
		
		String code;
		String name;
		String englishName;

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
			return this==English1?"en":code;
		}

	}
	
	static void Log(String name) {
		System.out.println(name);
	}
	
	static StringBuilder buf = new StringBuilder();
	static StringBuilder buf2 = new StringBuilder();
	static String[] last_paths;
	static String last_idF;
	static String last_id;
	static String last_nmF;
	static int last_st;

	private static void pushExtractAction(String idF, String id, String nmF, String...XMLPath) {
		Action actionExt=new Action(null, EXTRACT, XMLPath);
		actionExt.idField=idF;
		actionExt.id=id;
		actionExt.fieldName=nmF;
		actions.add(actionExt);
	}

	static LANG[] _filterOnShot;
	
	public static void extractItem(String idF, String id, String nmF, String...XMLPath) throws Exception {
		actions.clear();
		last_paths = XMLPath;
		last_idF = idF;
		last_id = id;
		last_nmF = nmF;
		last_st = buf.length();
		buf.append("{");
		pushExtractAction(idF, id, nmF, XMLPath);
		HashSet<LANG> filterOnShot = null;
		if (_filterOnShot!=null) {
			filterOnShot = new HashSet<>(Arrays.asList(_filterOnShot));
			_filterOnShot = null;
		}
		for(LANG idx:LANG.values()) {
			if (filterOnShot==null || filterOnShot.contains(idx)) {
				//Log("processing "+idx.code);
				processXmlFileByEnum(idx, true);
			}
		}
		int len = buf.length();
		if (len>2 && buf.charAt(len-2)==',') {
			buf.setLength(len-2);
		}
		
		buf.append("}\r\n");
	}

	@Test
	public void simpleExtractorTest() throws Exception {
		extractItem("id", "6103", "name", "Native-Langue", "Dialog", "Preference", "Global");
		//wrapLastExtractedItem("小图标", MODIFY);
		extractItem("id", "6104", "name", "Native-Langue", "Dialog", "Preference", "Global");
		//wrapLastExtractedItem("大图标", MODIFY);
		extractItem("id", "6105", "name", "Native-Langue", "Dialog", "Preference", "Global");
		//wrapLastExtractedItem("标准图标", MODIFY);
		//Log(buf.toString());
		Log(buf2.toString());
	}
}



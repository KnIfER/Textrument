package apply.translations;

import org.apache.commons.lang3.StringEscapeUtils;
import org.junit.Test;

import java.io.File;
import java.lang.reflect.Modifier;

import static apply.translations.TextrumentLocalePatchs.*;

public class Extractors_v1 {
	public static Object wrapStr(Object val) {
		return val==null?"null":val instanceof String?("\""+val+"\""):val;
	}

	public static void wrapLastExtractedItem(String comments, int action) throws Exception {
		wrapLastExtractedItem(comments, action, "Item");
	}
	
	public static void wrapLastExtractedItem(String comments, int action, String tagName) throws Exception {
		Object lastID = last_id;
		try {
			lastID = Integer.parseInt(last_id);
		} catch (NumberFormatException ignored) {  }
		buf2
				.append("//").append(comments).append("\n")
				.append("action = pushActionByPathFieldedIDForFieldInTag(\"")
				.append(buf.substring(last_st, buf.length()-2))
				.append("\", ")
				.append(actionGetName(action)).append(", ")
				.append( wrapStr(last_idF) ).append(", ")
				.append( wrapStr(lastID ) ).append(", ")
				.append( wrapStr(last_nmF) ).append(", ")
				.append( wrapStr(tagName) ).append(", ")
		;
		for (int j = 0; j < last_paths.length; j++) {
			buf2.append(wrapStr(last_paths[j]));
			if (j!=last_paths.length-1)
				buf2.append(", ");
		}
		buf2.append(");\n\n");
		//buf2.append("action.tagName=\"Item\";\n\n")
	}
	
	@Test
	public void test() throws Exception {
		sourceFolder=new File("D:\\Downloads\\npp.latest\\PowerEditor\\installer\\nativeLang");
		extractItem("CMID", "23", "name", "Native-Langue", "Menu", "TabBar");
		wrapLastExtractedItem("打开所在文件夹工作区", INSERT);
		buf2.append("action.id=\"24\"\n\n");
		Log(buf2.toString());
	}
	
	@Test
	public void extractFromNpp795() throws Exception {
		//extractItem("id", "6103", "name", "Native-Langue", "Dialog", "Preference", "Global"); // 小图标
		//extractItem("id", "6104", "name", "Native-Langue", "Dialog", "Preference", "Global"); // 大图标
		//extractItem("id", "6105", "name", "Native-Langue", "Dialog", "Preference", "Global"); // 标准图标
		sourceFolder=new File("D:\\Downloads\\npp.latest\\PowerEditor\\installer\\nativeLang");

		extractItem("CMID", "23", "name", "Native-Langue", "Menu", "TabBar");
		wrapLastExtractedItem("打开所在文件夹工作区", INSERT);
		buf2.append("action.id=\"24\";\n\n");

		Log(buf2.toString());
	}

	@Test
	public void extractFromNpp789finders() throws Exception {
		sourceFolder=new File("..\\PowerEditor\\installer\\nativeLang");

		extractItem(null, null, "value", "Native-Langue", "MiscStrings", "finder-copy");
		wrapLastExtractedItem("finder-copy", MODIFY);
		
		extractItem("id", "1721", "name", "Native-Langue", "Dialog", "Find");
		wrapLastExtractedItem("find prev", MODIFY);

		extractItem("id", "1723", "name", "Native-Langue", "Dialog", "Find");
		wrapLastExtractedItem("find next", MODIFY);

		Log(buf2.toString());
	}
	
	public void extractFromNpp789() throws Exception {
		sourceFolder=new File("..\\PowerEditor\\installer\\nativeLang");


		extractItem("id", "6206", "name", "Native-Langue", "Dialog", "Preference", "Scintillas");
		wrapLastExtractedItem("Display line number", MODIFY);
		
		extractItem("id", "6224", "name", "Native-Langue", "Dialog", "Preference", "Scintillas");
		wrapLastExtractedItem("Multi-Editing Settings", INSERT);
		extractItem("id", "6225", "name", "Native-Langue", "Dialog", "Preference", "Scintillas");
		wrapLastExtractedItem("Enable (Ctrl+Mouse click/selection)", MODIFY);
		
		
		extractItem("id", "6430", "name", "Native-Langue", "Dialog", "Preference", "DefaultDir");
		wrapLastExtractedItem("Use new style dialog", MODIFY);


		extractItem(null, null, "title", "Native-Langue", "Dialog", "Preference", "Cloud");
		wrapLastExtractedItem("Cloud", MODIFY);



		Log(buf2.toString());
	}


	@Test
	public void extractFromNpp789ToNpp795() throws Exception {
		// compare the updated string values.
		sourceFolder=new File("D:\\Downloads\\npp.latest\\PowerEditor\\installer\\nativeLang");


		
		Log(buf2.toString());
	}
	
	@Test
	public void extractTagsFromNpp795() throws Exception {
		sourceFolder=new File("D:\\Downloads\\npp.latest\\PowerEditor\\installer\\nativeLang");

		extractItem(null, "find-status-search-failed", "value", "Native-Langue", "MiscStrings"); // 

		extractItem(null, "finder-find-in-finder", "value", "Native-Langue", "MiscStrings"); // 
		extractItem(null, "finder-close", "value", "Native-Langue", "MiscStrings"); // 

		extractItem(null, "splitter-rotate-left", "value", "Native-Langue", "MiscStrings"); // 
		extractItem(null, "splitter-rotate-right", "value", "Native-Langue", "MiscStrings"); // 

		extractItem(null, "find-in-files-progress-title", "value", "Native-Langue", "MiscStrings"); // 
		extractItem(null, "replace-in-files-progress-title", "value", "Native-Langue", "MiscStrings"); // 
		extractItem(null, "replace-in-projects-confirm-title", "value", "Native-Langue", "MiscStrings"); // 
		extractItem(null, "replace-in-projects-confirm-message", "value", "Native-Langue", "MiscStrings"); // 

		extractItem(null, "find-result-caption", "value", "Native-Langue", "MiscStrings"); // 
		extractItem(null, "tab-untitled-string", "value", "Native-Langue", "MiscStrings"); // 


		Log(buf.toString());
	}
}

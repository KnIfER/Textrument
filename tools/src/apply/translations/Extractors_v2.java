package apply.translations;

import org.junit.Test;

import java.io.File;

import static apply.translations.TextrumentLocalePatchs.*;
import static apply.translations.Extractors_v1.*;

public class Extractors_v2 {
	
	@Test
	public void extractFromNpp795ALTICO() throws Exception {
		sourceFolder=new File("D:\\Downloads\\npp.latest\\PowerEditor\\installer\\nativeLang");

		extractItem("id", "6128", "name", "Native-Langue", "Dialog", "Preference", "Global");
		wrapLastExtractedItem("自定义图标", INSERT);
		buf2.append("action.id=\"6129\";\n\n");

		Log(buf2.toString());
	}

}

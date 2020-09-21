package apply.translations;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.channels.FileChannel;

import static apply.translations.TextrumentLocalePatchs.*;

public class Install {
	static File targetFolder=new File("..\\PowerEditor\\bin64\\localization");
	static File targetFolder1=new File("..\\PowerEditor\\bin64\\User\\nativeLang.xml");

	public static void main(String[] args) throws IOException {
		File[] fileArr = sourceFolder.listFiles();
		for (int i = 0; i < fileArr.length; i++) {
			installXmlTo(fileArr[i], new File(targetFolder, fileArr[i].getName()));
		}
	}

	public static void installXmlTo(File from, File to) throws IOException {
		Log(from.getPath());
		FileChannel fChanIn = new FileInputStream(from).getChannel();
		FileChannel fChanOut = new FileOutputStream(to).getChannel();
		fChanIn.transferTo(0, from.length(), fChanOut);
		fChanIn.close();
		fChanOut.close();
	}
	
	public static void installXmlByLocale(LANG Enum) throws IOException {
		int id = shortName_id_table.get(Enum.code);
		String xmlFileName = filters[id];
		File file = new File(sourceFolder, xmlFileName);
		if(file.exists()) {
			installXmlTo(file, new File(targetFolder, file.getName()));
		}
	}
	
	public static void installXmlNativeByLocale(LANG Enum) throws IOException {
		int id = shortName_id_table.get(Enum.code);
		String xmlFileName = filters[id];
		File file = new File(sourceFolder, xmlFileName);
		if(file.exists()) {
			installXmlTo(file, targetFolder1);
		}
	}
}

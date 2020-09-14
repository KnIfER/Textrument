package apply.translations;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.channels.FileChannel;

import static apply.translations.TextrumentLocalePatchs.Log;
import static apply.translations.TextrumentLocalePatchs.sourceFolder;

public class Install {
	static File targetFolder=new File("..\\PowerEditor\\bin64\\localization");

	public static void main(String[] args) throws IOException {
		File[] fileArr = sourceFolder.listFiles();
		for (int i = 0; i < fileArr.length; i++) {
			Log(fileArr[i].getPath());
			FileChannel fChanIn = new FileInputStream(fileArr[i]).getChannel();
			FileChannel fChanOut = new FileOutputStream(new File(targetFolder, fileArr[i].getName())).getChannel();
			fChanIn.transferTo(0, fileArr[i].length(), fChanOut);
			fChanIn.close();
			fChanOut.close();
		}
	}
	
	
}

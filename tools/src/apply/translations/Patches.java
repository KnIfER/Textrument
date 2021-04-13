package apply.translations;

public class Patches {
	public static void pushAll() {
		new Patch_V1().pushActions();
		new Patch_V2().pushActions();
		new Patch_V3().pushActions();
		new Patch_V4().pushActions();
	}
}

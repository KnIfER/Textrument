package apply.translations;

import static apply.translations.TextrumentLocalePatchs.*;


// update from Npp789 -> Npp795
public class Patch_V4 implements ActionApplier {
	Action action;
	@Override
	public void pushActions() {
		//action = pushActionByPathFieldedIDForFieldInTag("{zh_CN:'打开文件夹工作区'}", MODIFY, "CMID", 24, "name", "Item", "Native-Langue", "Menu", "TabBar");

//finder-copy
		action = pushActionByPathFieldedIDForFieldInTag("{ar:'نسخ', ga:'Cóipeáil', be:'Капіяваць', bg:'Копиране', pl:'Kopiuj', da:'Kopier', de:'Kopieren', ru:'Копировать', fr:'Copier', fi:'Kopioi', ko:'복사', nl:'Kopiëren', ca:'Copia', cs:'Kopírovat', co:'Cupià', hr:'Kopiraj', lt:'Kopijuoti', ro:'Copiere', zu:'Kopisha', ne:'कापी', ja:'コピー', sv:'Kopiera', sk:'Kopírovať', sl:'Kopiraj', tr:'Kopyala', uk:'Копіювати', uz:'Nusxalamoq', es:'Copiar', hu:'Másolás', it:'Copia', hi:'कॉपी करें', id:'Salin', en:'Copy', en1:'Copy', zh_CN:'复制', bre:'Eilañ', venetian:'Còpia', piglatin:'Opycay', taiwanese:'複製', pt_b:'Copiar'}", MODIFY, null, null, "value", "finder-copy", "Native-Langue", "MiscStrings", "finder-copy");
		
		action = pushActionByPathFieldedIDForFieldInTag(null, REMOVECONTENTS, null, null, "value", "finder-copy", "Native-Langue", "MiscStrings", "finder-copy");

//find prev
		action = pushActionByPathFieldedIDForFieldInTag("{ar:'▲', ga:'▲', be:'▲', bg:'▲', pl:'▲', da:'▲', de:'▲', ru:'▲', fr:'▲', fi:'▲', ko:'▲', nl:'▲', ca:'▲', cs:'▲', co:'▲', hr:'▲', ne:'▲', ja:'▲', sv:'▲', sk:'▲', sl:'▲', tr:'▲', uk:'▲', es:'▲', hu:'▲', it:'▲', hi:'▲', en:'▲ Previous', en1:'▲ Previous', zh_CN:'▲ 上一个', bre:'▲', venetian:'▲', hongKongCantonese:'▲ 上一個', taiwanese:'▲ 上一個', pt_b:'▲'}", MODIFY, "id", 1721, "name", "Item", "Native-Langue", "Dialog", "Find");

//find next
		action = pushActionByPathFieldedIDForFieldInTag("{ar:'▼ التالي', ga:'▼ Aghaidh', be:'▼ Далей', bg:'▼ Следващ', pl:'▼ Następne', da:'▼ næste', de:'▼ finden', ru:'▼', fr:'▼ Suivant', fi:'▼ Seuraava', ko:'▼ 다음', nl:'▼ Volgende', ca:'▼ Següent', cs:'▼ Další', co:'▼ Seguente', hr:'▼ Sljedeće', ne:'▼अर्को', ja:'▼ 次を検索', sv:'▼ Nästa', sk:'▼ Next', sl:'▼ Naprej', tr:'▼ Sonrakini', uk:'▼ Далі', es:'▼ Siguiente', hu:'▼ Következő', it:'▼ Successivo', hi:'▼ अगल', en:'▼ Next', en1:'▼ Next', zh_CN:'▼ 下一个', bre:'▼ da-heul', venetian:'▼ Seguente', hongKongCantonese:'▲ 下一個', taiwanese:'▼ 下一個', pt_b:'▼ Próximo'}", MODIFY, "id", 1723, "name", "Item", "Native-Langue", "Dialog", "Find");

		
		action = pushActionByPathFieldedIDForFieldInTag(null, RENAME, null, null, null, null);
		action.tagName = "Textrument";

		
		action = pushActionByPathFieldedIDForFieldInTag(null, MODIFY, null, null, "version", "Native-Langue", "Native-Langue");
		action.converter = input -> "v1.0.0";

		//自定义图标
		action = pushActionByPathFieldedIDForFieldInTag("{bg:'Алтернативни икони', pl:'Alternatywne ikony', de:'Alternative Symbole', ru:'Альтернативные иконки', ko:'대체 아이콘', nl:'Alternatieve pictogrammen', cs:'Alternativní ikony', co:'Icone alternative', ja:'別デザインのアイコンを使用する', sk:'Alternatívne ikony', sl:'Alternate icons', tr:'Alternatif Simgeler', uk:'Змінювати піктограми', es:'Iconos alternados', it:'Alterna icone', en:'Alternate icons', en1:'Alternate icons', zh_CN:'另一套图标', venetian:'Alterna icone', hongKongCantonese:'另類圖示', abkhazian:'Ихархә. Альтернативатә дыргаҷқәа'}", INSERT, "id", 6129, "name", "Item", "Native-Langue", "Dialog", "Preference", "Global");

		//自定义图标
		action = pushActionByPathFieldedIDForFieldInTag("{venetian:'',abkhazian:''}", DELETE, "id", 6128, "name", "Item", "Native-Langue", "Dialog", "Preference", "Global");

	}
}

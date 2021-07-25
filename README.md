![](../master/PowerEditor/src/icons/npp.ico)何为图创文本 ? | WHAT
===================
Text（`文本`） + Instrument（`器具`） = Textrument（`图创文本`）

根据自己的理解和需求，我对旧产品进行了各种升级和改造，并愿以此铸造文字之器。

日常使用推荐编辑器之王vscode，本产品胜在轻量小巧、定制方便，一切皆有可能。

与旧产品有何区别? | WHY
===================
1. 修改了文件保存方式，旧产品在磁盘写满时，会写入空文件，可能导致数据丢失。图创文本鉴于此加以改进，能写多少数据就写多少数据，保护数据毫不让步。原理是用r+标志打开文件。 | **Value** you data by preventing NULL corruption.

2. 升级可停靠窗口。预览方式升级为显示透明窗口，区别于旧产品直接在桌面画布上进行绘制。窗口布局保存在会话(session)中，因此不同的会话可有不同的界面布局。 | Updated **Docking** / Preview System.

3. 放大设置面本、搜索框等常用界面的字体（可选），使用更轻松。 | Larger Fonts(WIP).

4. 宏录制与回放更方便。“重复运行宏”可遍历打开的所有文件。右击相关菜单与工具栏按钮可触发更多功能。 | Easier access to recorded **marcos** via Right-Click Context Menu on the Toolbar button.

5. 为其他工具栏按钮增加右键菜单。 | Other **Right-Click Context Menu** on the Toolbar buttons( WIP：  plugins, increment search, zoom, ...)

6. 优化滚动条的使用体验。已有功能：shift+鼠标点击=快速滚动，shift+鼠标滚轮=横向滚动，在此基础上，支持一边拖拽横向滚动条，一边使用鼠标滚轮进行垂直滚动。 | **Better horizontal scrollbar**.

7. 完善 Scintilla 组件，让其同时支持IDEA式多行列选模式，与传统的方形列选模式。两种列选择模式的切换快捷键为 Alt+Shift+C。 | **IDEA**-favored **Column Selection Mode**.

8. 编码种增设二进制模式。此模式下不转义一些特殊字符，可以很好地配合Hex Editor插件。 | Binary encoding mode.


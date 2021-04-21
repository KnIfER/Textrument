
#include "CustomFileDialog.h"


CustomFileDialog::CustomFileDialog(HWND hwnd)
{

}

CustomFileDialog::~CustomFileDialog()
{

}

void CustomFileDialog::setTitle(const TCHAR* title)
{

}

void CustomFileDialog::setExtFilter(const TCHAR* text, const TCHAR* ext)
{

}

void CustomFileDialog::setExtFilter(const TCHAR* text, std::initializer_list<const TCHAR*> exts)
{

}

void CustomFileDialog::setDefExt(const TCHAR* ext)
{

}

void CustomFileDialog::setDefFileName(const TCHAR *fn)
{

}


void CustomFileDialog::setFolder(const TCHAR* folder)
{

}

void CustomFileDialog::setCheckbox(const TCHAR* text, bool isActive)
{

}


void CustomFileDialog::setExtIndex(int extTypeIndex)
{

}

generic_string CustomFileDialog::doSaveDlg()
{
	return L"";
}

generic_string CustomFileDialog::pickFolder()
{
	return L"";
}

generic_string CustomFileDialog::doOpenSingleFileDlg()
{
	return L"";
}

std::vector<generic_string> CustomFileDialog::doOpenMultiFilesDlg()
{
	return {};
}

bool CustomFileDialog::getCheckboxState() const
{
	return false;
}

bool CustomFileDialog::isReadOnly() const
{
	return false;
}

class CustomFileDialog::Impl
{
public:
	Impl() = default;

	~Impl() = default;
private:
	int xx;
};
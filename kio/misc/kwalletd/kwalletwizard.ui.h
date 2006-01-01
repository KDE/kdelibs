/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void KWalletWizard::passwordPageUpdate()
{
    bool fe = !_useWallet->isChecked() || _pass1->text() == _pass2->text();
    if (_basic->isChecked()) {
        setFinishEnabled(page2, fe);
    } else {
        setNextEnabled(page2, fe);
        setFinishEnabled(page3, fe);
    }
    
    if (_useWallet->isChecked()) {
	if (_pass1->text() == _pass2->text()) {
            if (_pass1->text().isEmpty()) {
                _matchLabel->setText(i18n("<qt>Password is empty.  <b>(WARNING: Insecure)"));
            } else {
                _matchLabel->setText(i18n("Passwords match."));
            }
	} else {
	    _matchLabel->setText(i18n("Passwords do not match."));
	}
    } else {
	_matchLabel->setText(QString());
    }
    
}


void KWalletWizard::init()
{
    setHelpEnabled(page1, false);
    setHelpEnabled(page2, false);
    setHelpEnabled(page3, false);
    setHelpEnabled(page4, false);
    setAppropriate(page3, false);
    setAppropriate(page4, false);
    setFinishEnabled(page2, true);
}


void KWalletWizard::setAdvanced()
{
    setAppropriate(page3, true);
    setAppropriate(page4, true);
    bool fe = !_useWallet->isChecked() || _pass1->text() == _pass2->text();
    setFinishEnabled(page2, false);
    setNextEnabled(page2, fe);
    setFinishEnabled(page3, fe);
}


void KWalletWizard::setBasic()
{
    setAppropriate(page3, false);
    setAppropriate(page4, false);
    bool fe = !_useWallet->isChecked() || _pass1->text() == _pass2->text();
    setFinishEnabled(page3, false);
    setFinishEnabled(page2, fe);
}


void KWalletWizard::destroy()
{
    _pass1->clear();
    _pass2->clear();
}

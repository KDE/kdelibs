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
    bool fe = true;
    if (_basic->isChecked()) {
	fe = !_useWallet->isChecked() || (_pass1->text() == _pass2->text());
	fe = !_pass1->text().isEmpty() && fe;
        setFinishEnabled(page2, fe);
    } else {
	fe = !_useWallet->isChecked() || (_pass1->text() == _pass2->text());
	fe = !_pass1->text().isEmpty() && fe;
        setFinishEnabled(page3, fe);
    }
    
    if (_useWallet->isChecked()) {
	if (_pass1->text() == _pass2->text()) {
	    _matchLabel->setText(tr("Passwords match."));
	} else {
	    _matchLabel->setText(tr("Passwords do not match."));
	}
    } else {
	_matchLabel->setText(QString::null);
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
}


void KWalletWizard::setAdvanced()
{
    setAppropriate(page3, true);
    setAppropriate(page4, true);
}


void KWalletWizard::setBasic()
{
    setAppropriate(page3, false);
    setAppropriate(page4, false);
}


void KWalletWizard::destroy()
{
    _pass1->clear();
    _pass2->clear();
}

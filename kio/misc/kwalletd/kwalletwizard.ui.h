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
    } else {
	fe = !_useWallet->isChecked() || (_pass1->text() == _pass2->text());
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
    
    setFinishEnabled(page2, fe);
}

package org.kde.kjas.server;

import java.applet.Applet;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.LayoutManager;
import java.awt.Panel;
import java.net.URL;

/**
 * @author till
 *
 * A panel which embeds the applet and shows some
 * information during class loading.
 */
public class KJASAppletPanel extends javax.swing.JPanel implements StatusListener {

	private Dimension size;
	private Image img = null;
	private boolean showStatusFlag = true;
	private Font font;
	private String msg = "Loading Applet...";
	
	/**
	 * Constructor for KJASAppletPanel.
	 */
	public KJASAppletPanel(Dimension _size) {
		super(new BorderLayout());
		size = _size;
		font = new Font("SansSerif", Font.PLAIN, 10);
		URL url =
			getClass().getClassLoader().getResource("images/animbean.gif");
		img = getToolkit().createImage(url);
		//setBackground(Color.white);
	}

	void setAppletSize(Dimension _size) {
		size = _size;
	}

	public Dimension getPreferredSize() {
		return size;
	}

	public Dimension getMinimumSize() {
		return size;
	}

	void setApplet(Applet applet) {
		add("Center", applet);
		validate();
	}

	public void showStatus(String msg) {
		this.msg = msg;
		repaint();
	}

	public void paint(Graphics g) {
		super.paint(g);
		if (showStatusFlag) {
			int x = getWidth() / 2;
			int y = getHeight() / 2;
			if (img != null) {
				//synchronized (img) {
				int w = img.getWidth(this);
				int h = img.getHeight(this);
				int imgx = x - w / 2;
				int imgy = y - h / 2;
				//g.setClip(imgx, imgy, w, h);
				g.drawImage(img, imgx, imgy, this);
				y += img.getHeight(this) / 2;
				//}
			}
			if (msg != null) {
				//synchronized(msg) {
				g.setFont(font);
				FontMetrics m = g.getFontMetrics();
				int h = m.getHeight();
				int w = m.stringWidth(msg);
				int msgx = x - w / 2;
				int msgy = y + h;
				//g.setClip(0, y, getWidth(), h);
				g.drawString(msg, msgx, msgy);

				//}
			}
		}
	}
	void showFailed() {
		URL url =
			getClass().getClassLoader().getResource("images/brokenbean.gif");
		img = getToolkit().createImage(url);
		msg = "Applet Failed.";
		repaint();
	}

	void showFailed(String message) {
		showFailed();
		showStatus(message);
	}

	public void stopAnimation() {
		showStatusFlag = false;
	}

}

/**
 *
 */
package com.absence.chiara.MoodLed;

import java.awt.AWTException;
import java.awt.BorderLayout;
import java.awt.Image;
import java.awt.SystemTray;
import java.awt.TrayIcon;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JColorChooser;
import javax.swing.JFrame;

/**
 * @author chiara@absence.it
 *
 */
@SuppressWarnings("serial")
public class MoodLed extends JFrame implements ActionListener, WindowListener {

	private final JColorChooser cp;
	private final ScreenAnalyzer sa;
	private final JButton b;
	private final SystemTray t;
	private final TrayIcon ti;
	private final Timer timer;

	public MoodLed() {
		super(String.format("%s v%s", Properties.ProgramName, Properties.CurrentVersion));

		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		addWindowListener(this);

		BorderLayout layout = new BorderLayout();
		setLayout(layout);

		sa = new ScreenAnalyzer();
		cp = new JColorChooser();
		b = new JButton("Refresh", new ImageIcon(getClass().getResource("/com/absence/chiara/MoodLed/pictures/icon_24.png")));

		List<Image> icons = new ArrayList<Image>();
		icons.add(new ImageIcon(getClass().getResource("/com/absence/chiara/MoodLed/pictures/icon_16.png")).getImage());
		icons.add(new ImageIcon(getClass().getResource("/com/absence/chiara/MoodLed/pictures/icon_24.png")).getImage());
		icons.add(new ImageIcon(getClass().getResource("/com/absence/chiara/MoodLed/pictures/icon_32.png")).getImage());
		icons.add(new ImageIcon(getClass().getResource("/com/absence/chiara/MoodLed/pictures/icon_48.png")).getImage());
		icons.add(new ImageIcon(getClass().getResource("/com/absence/chiara/MoodLed/pictures/icon_64.png")).getImage());
		setIconImages(icons);

		// https://docs.oracle.com/javase/tutorial/uiswing/misc/systemtray.html
		t = SystemTray.getSystemTray();
		ti = new TrayIcon(new ImageIcon("/com/absence/chiara/MoodLed/pictures/icon_24.png").getImage());

		try {

			t.add(ti);
			ti.setToolTip(Properties.ProgramName);
		} catch (AWTException e) {

			e.printStackTrace();
		}

		setSize(300, 300);
		
		LEDManager.init();
		LEDManager.switchOn();
		
		cp.setColor(sa.getPredominantColor().getColor());
		b.addActionListener(this);

		getContentPane().add(cp, BorderLayout.CENTER);
		getContentPane().add(b, BorderLayout.SOUTH);

		timer = new Timer(true);
		timer.scheduleAtFixedRate(new TimerTask() {

			@Override
			public void run() {

				updateColor();
			}
		}, 0L, 2000);
    }

	/**
	 * @param args
	 */
	public static void main(String[] args) {

		MoodLed m = new MoodLed();

		m.pack();
		m.setVisible(true);
	}

	private void updateColor() {
	    Colors c = sa.getPredominantColor();

		cp.setColor(c.getColor());
		ti.setToolTip(c.getMood());
		// LEDManager.setRGB(c.getColor().getRed(), c.getColor().getGreen(), c.getColor().getBlue());
		// ti.displayMessage(null, c.getMood(), MessageType.INFO);

		System.out.println(String.format("%d:%d:%d %s", c.getColor().getRed(), c.getColor().getGreen(), c.getColor().getBlue(), c.getMood()));
	}


	@Override
    public void actionPerformed(ActionEvent e) {

		updateColor();
    }

	@Override
    public void windowOpened(WindowEvent e) { }

	@Override
    public void windowClosing(WindowEvent e) {

		LEDManager.release();
	    timer.cancel();
    }

	@Override
    public void windowClosed(WindowEvent e) { }

	@Override
    public void windowIconified(WindowEvent e) { }

	@Override
    public void windowDeiconified(WindowEvent e) { }

	@Override
    public void windowActivated(WindowEvent e) { }

	@Override
    public void windowDeactivated(WindowEvent e) { }
}

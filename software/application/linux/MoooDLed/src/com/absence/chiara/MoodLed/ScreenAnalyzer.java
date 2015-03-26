/**
 *
 */
package com.absence.chiara.MoodLed;

import java.awt.AWTException;
import java.awt.Color;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Rectangle;
import java.awt.Robot;
import java.awt.image.BufferedImage;

/**
 * @author chiara@absence.it
 *
 */
public class ScreenAnalyzer {

	Robot r;

	public ScreenAnalyzer() {

		 try {
			r = new Robot();

		} catch (AWTException e) {

			e.printStackTrace();
		}
	}

	public Colors getPredominantColor() {

		GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();

		for (GraphicsDevice monitor : ge.getScreenDevices()) {
			getMonitorPredominantColor(monitor);
		}

		return Colors.getPredominantColor();
	}

	private void getMonitorPredominantColor(GraphicsDevice monitor) {

		int height = monitor.getDisplayMode().getHeight();
		int width = monitor.getDisplayMode().getWidth();
		int x = monitor.getDefaultConfiguration().getBounds().x;
		int y = monitor.getDefaultConfiguration().getBounds().y;

		Rectangle monitorRect = new Rectangle(x, y, width, height);
		BufferedImage img = r.createScreenCapture(monitorRect);
/*
		File file = new File(String.format("screencapture_%d.jp", x));
		try {
			ImageIO.write(img, "jpg", file);
			System.out.println(file.getAbsolutePath());
		} catch (IOException e) {
			e.printStackTrace();
		}
*/
		Color c;

		for (int xCoord = 0; xCoord < img.getWidth(); xCoord += 1) {
			for (int yCoord = 0; yCoord < img.getHeight(); yCoord += 1) {

				// get the color of the pixel
				c = new Color(img.getRGB(xCoord, yCoord));
				Colors.count(c);
			}
		}
	}
}

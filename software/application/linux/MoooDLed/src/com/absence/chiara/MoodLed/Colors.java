package com.absence.chiara.MoodLed;

import java.awt.Color;
import java.util.Arrays;
import java.util.Collections;

public enum Colors {

	WHITE	(Color.WHITE, 0, "clarity"),
	GRAY	(Color.GRAY, 0, "50 shades of boredom"),
	BLACK	(Color.BLACK, 0, "tide of darkness"),

//	H_000(Color.RED, 0), // 10
	H_010(Color.RED, 0, "aggressiveness"),
//	H_020(Color.GRAY, 0),
	H_030(Color.ORANGE.darker(), 0, "vigilance"),
//	H_040(Color.GRAY, 0),
	H_050(Color.YELLOW, 0, "ecstasy"),
//	H_060(Color.GRAY, 0),
	H_070(Color.GREEN.brighter(), 0, "serenity"), // 80
//	H_080(Color.GRAY, 0),
	H_090(Color.GREEN, 0, "trust"),
//	H_100(Color.GRAY, 0),
	H_110(Color.GREEN.darker(), 0, "admiration"),
//	H_120(Color.GRAY, 0),
	H_130(Color.GRAY, 0, "fear"),
//	H_140(Color.GRAY, 0),
	H_150(Color.GRAY, 0, "apprehension"),
//	H_160(Color.GRAY, 0),
	H_170(Color.CYAN, 0, "distraction"), // 180
//	H_180(Color.GRAY, 0),
	H_190(Color.BLUE.brighter(), 0, "surprise"),
//	H_200(Color.GRAY, 0),
	H_210(Color.BLUE, 0, "sadness"),
//	H_220(Color.GRAY, 0),
	H_230(Color.BLUE, 0, "grief"),
//	H_240(Color.GRAY, 0),
	H_250(Color.GRAY, 0, "loathing"),
//	H_260(Color.GRAY, 0), // 270
	H_270(Color.MAGENTA.darker(), 0, "rage"),
//	H_280(Color.GRAY, 0),
	H_290(Color.MAGENTA, 0, "anger"),
//	H_300(Color.GRAY, 0),
	H_310(Color.MAGENTA.brighter(), 0, "contempt"),
//	H_320(Color.GRAY, 0),
	H_330(Color.PINK.darker(), 0, "annoyance"),
//	H_340(Color.GRAY, 0),
	H_350(Color.RED.brighter(), 0, "aggressiveness");

	private Color color;
	private Integer count;
	private final String mood;

	Colors(Color color, int count, String mood) {
		this.color = color;
		this.count = count;
		this.mood = mood;
	}

	public static void count(Color rgbColor) {

		float h = 0;
		float s = 0;
		float b = 0;
		float[] hsb = {0, 0, 0};

		Color.RGBtoHSB(rgbColor.getRed(), rgbColor.getGreen(), rgbColor.getBlue(), hsb);

		h = (hsb[0]*360);
		s = (hsb[1]*100);
		b = (hsb[2]*100);

		if (b > 97)
			WHITE.count++;
		else if (b < 5)
			BLACK.count++;
		else if (s < 5)
			GRAY.count++;
		else {

			Colors c = Colors.values()[(int) (h / 20 + 3)];
			c.count++;
			c.color = new Color(
					(c.color.getRed() + rgbColor.getRed())/2,
					(c.color.getGreen() + rgbColor.getGreen())/2,
					(c.color.getBlue() + rgbColor.getBlue())/2);
		}
	}

	public static Colors getPredominantColor() {
/*
		StringBuilder s = new StringBuilder();
		int pixelCount = 0;

		for (Colors c : Colors.values()) {

			pixelCount += c.getCount();
			s.append(String.format("%s\t = %d;\n", c.toString().toLowerCase(), c.getCount()));
		}

		System.out.println(String.format("%s\nTotal count:\t %d\nExpected:\t 4147200", s.toString(), pixelCount));
*/
		Colors max = Collections.max(Arrays.asList(Colors.values()), new ColorComparator());
		resetCount();

		return max;
	}

	private static void resetCount() {

		for (Colors c : Colors.values())
			c.count = 0;
	}

	public Color getColor() {
	    return color;
    }

	public Integer getCount() {
	    return count;
    }

	public String getMood() {
	    return mood;
    }
}


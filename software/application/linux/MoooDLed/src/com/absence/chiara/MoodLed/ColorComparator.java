/**
 *
 */
package com.absence.chiara.MoodLed;

import java.util.Comparator;

/**
 * @author chiara@abence.it
 *
 */

public class ColorComparator implements Comparator<Colors> {

	@Override
	public int compare(Colors c0, Colors c1) {

		return c0.getCount().compareTo(c1.getCount());
	}
}

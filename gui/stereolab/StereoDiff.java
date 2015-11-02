package stereolab;

/* 
 ** Klasse:      StereoDiff
 ** Autor:       Christian Werner <cwerner@informatik.hu-berlin.de>
 ** Version:     1.0 (vom 22. April 2002)
 **
 ** Beschreibung:
 **
 ** Algorithmenklasse für das Projekt "StereoLab". Flächenbasierter Algorithmus
 ** zur Disparitätenberechnung, der absolute Grauwertdifferenzen als
 ** Ahnlichkeitskriterium benutzt.
 */

import javax.swing.*;

public class StereoDiff extends StereoAlgorithm {

	public StereoDiff(int B, int H, int TauMax, boolean U, int S, boolean F) {
		super(B, H, TauMax, U, S, F);
	}

	public String toString() {
		String st = "Algorithmus:              Betrag der Diff.\n"
				+ "Fenstergöße:              " + String.valueOf(b) + " x "
				+ String.valueOf(h) + "\n" + "Tau max:                  "
				+ String.valueOf(tauMax) + "\n" + "Schwellwert/Fenster-Fkt.: "
				+ (useS ? "ja" : "nein") + "/" + (useF ? "ja" : "nein");
		if (useS)
			st = st + "\nSchwellwert:              " + String.valueOf(s);
		return (st);
	}

	public RangeProfile calc(byte[][] lb, byte[][] rb, TimerProgressBar p) {

		System.out.println("Starting StereoDiff calculation...");
		long start = System.nanoTime();

		int zeilen = lb.length;
		int spalten = lb[0].length;
		int xu = -b / 2;
		int xo = (b % 2 == 1) ? b / 2 : b / 2 - 1;
		int yu = -h / 2;
		int yo = (h % 2 == 1) ? h / 2 : h / 2 - 1;
		int wc = b / 2 + h / 2 + 1; // Eintrag in der "Mitte" der Fensterfkt.
		int optIndex;
		int optWert;
		int wert;
		boolean val;

		RangeProfile profile = new RangeProfile(zeilen, spalten);
		for (int i = 0; i < zeilen; i++) {
			p.setValue(i);
				for (int j = 0; j < spalten; j++) {

				if ((i + yu < 0) | (i + yo >= zeilen) | (j + xu - tauMax < 0)
						| (j + xo + tauMax >= spalten)) {
					profile.setProfile(i, j, 0);
					profile.setValid(i, j, false);
				} else {
					optWert = Integer.MAX_VALUE;
					optIndex = 0;
					val = false;
					for (int tau = -tauMax; tau <= tauMax; tau++) {
						wert = 0;
						for (int k = xu; k <= xo; k++) {
							for (int l = yu; l <= yo; l++) {
								wert = wert
								+ Math.abs(StereoToolkit
										.unsignedByteToInt(lb[i + l][j
												+ k])
										- StereoToolkit
												.unsignedByteToInt(rb[i
														+ l][j + k
														+ tau]))
								* (useF ? wc - Math.abs(l)
										- Math.abs(k) : 1);
							}
						}
						if (wert < optWert) {
							optWert = wert;
							optIndex = tau;
							val = true;
						} else if (wert == optWert) {
							val = false;
						}
					}
					if ((useS) && (optWert > s))
						val = false;
					profile.setProfile(i, j, optIndex);
					profile.setValid(i, j, val);
				}
			}
		}
		
		long stop = System.nanoTime();

		System.out.println("Finished StereoDiff calculation... (took "
				+ (stop - start) / 1000000.0 + " ms)");

		return (profile);

	}

}

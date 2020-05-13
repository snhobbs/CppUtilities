from matplotlib import pyplot as plt, style
import numpy as np
from math import exp, pi, sqrt, atan
import scipy as sp
from scipy import signal

'''
Our voltage to current is 0V = -2.25A, 3.3V = +2.25A ADC range 0-2^12 Plant characterization given a 1.8A step input, so our integrator gain is that slope in (V/S) / (A) * (A/V) giving (V/V)/S which is the units of our integrator gain and is seperate from either voltage or ADC units. As the integrator gain is in arbitrary units, the tranlation to volts is not required
'''
#Controller Description
H = lambda x : sp.heaviside(x, 0.5)

class System(object):
    def __init__(self, plant, update_rate):
        self.update_rate = update_rate
        self.plant = plant
        self.slope = plant.slope
        self.delay = plant.delay
        self.kp = 1
        self.ti = 2
        self.simcTuning()

        #self.kp = 1./38
        #self.kp = 1./32
        #self.ti = 8.
        self.ki = self.kp/self.ti

    def znTuning(self):
        alpha = 0.714#tuning perameter Ziegler-Nichols
        beta = 3.33#tuning perameter Ziegler-Nichols
        self.kp = alpha/(self.slope*self.delay)
        self.ti = beta*self.delay


    def simcTuning(self):
        alpha = 16#16
        beta = 0.4
        self.kp = beta*1/(2*self.slope*self.delay)
        self.ti = alpha*self.delay

    def invRespTuning(self):
        c = 2.75
        self.ti = (2*c+1)*self.delay
        self.kp = (2*c+1)/(self.slope*self.delay*(c+1)**2)

    def transferOpenLoop(self, s):
        k = self.slope*self.kp
        ti = self.ti
        return k*exp(-self.delay*s)*(ti*s + 1)/(ti*s**2)

    def transferClosedLoop(self, s):
        k = self.slope*self.kp
        ti = self.ti
        G_s = k*exp(-self.delay*s)*(ti*s + 1)/(ti*s**2)
        return G_s/(1+G_s)

    def lti(self):
        k = self.slope*self.kp
        ti = self.ti
        return signal.lti([k*ti, k], [ti, 0, 0])#k*(s + ti)/(ti*s**2)

    def makeBode(self, w = None):
        system = self.lti()
        omega, mag, phase = signal.bode(system, w = w)
        phase = [p - ((180/pi)*w*self.delay) for p, w in zip(phase, omega)]
        return omega, mag, phase

    def findZero(self, arr):
        for i, a in enumerate(arr):
            if a <= 0:
                return i
        return None

    def phaseMargin(self):
        omega, mag, phase = self.makeBode()
        freq = [w/(2*pi) for w in omega]
        interset = self.findZero(mag)

        pm = 180 + phase[interset]

        return max(phase)+180, pm, freq[interset]

#Description of the plant
class Plant(object):
    def __init__(self, delay, slope):
        self.delay = float(delay)#delay time in seconds
        self.slope = float(slope)#process/time/control

    def transfer(self, s):
        return self.slope*exp(-s*self.delay)/s

def LoadTestData(fname):
    time, sig = np.loadtxt(fname, delimiter = ',', skiprows = 1, unpack=True)
    for i, t in enumerate(time):
        if t > 0:
            offset = i
            break
    return time, sig, offset


def findMultShift(val):
    '''
    mult>>shift ~= val -> val/mult = 1/2**shift -> s**shift = mult/val -> shift = log_2(mult/val)
    calc mult for the diffent shifts and return the pair with the smallest error
    mult = 2**shift * val
    '''
    val = float(val)
    error = val
    multiplier = 1
    shift = 0

    for i in range(0,14):
        for d in [-2,-1,0,1,2]:
            mult = round(2**i*val) + d
            if(mult <=0): continue

            value = float(mult)/(2**i)
            err = (val - value)
            if abs(err) < abs(error):
                #print(err, error, shift, mult)
                error = err
                shift = i
                multiplier = mult
    return multiplier, shift, error

def CalcPositionOnLine(Slope, Intercept, delay, x):
    return (x-delay)*Slope + Intercept


def FitInitialY(x, y, fitpoints = 100):
    '''
    Take the inital Y as the offset in a horizontal line
    fit with the first few points, nominally 100
    '''
    def HorizLine(x, y):
        return y

    popt, pcov = sp.optimize.curve_fit(HorizLine, x[:fitpoints], y[:fitpoints], p0=[y[0]], sigma=None, absolute_sigma=False, check_finite=True, bounds=(0, 20), method=None, jac=None)
    initialY = popt[0]
    return round(initialY, 4)

def BoxCarData(x, size):
    '''
    return left boxcar of data set
    '''
    out = np.zeros(int(len(x)/size))

    for i, _ in enumerate(out):
        out[i] = sum(x[i*size:(i+1)*size])
    print(out)

def DelayIntegrator(x, delay, slope):
    global start_y;
    out = []
    for pt in x:
        if(pt < delay):
            out.append(start_y)
        else:
            out.append(CalcPositionOnLine(slope, start_y, delay, pt))
    return out


def FitDelayIntegrator(x, y, DelaySeed, SlopeSeed, starty):
    #bounds = (-100, 100)
    bounds = (-100e6, 100e6)
    global start_y;
    start_y = starty
    popt, pcov = sp.optimize.curve_fit(DelayIntegrator, x, y, p0=[DelaySeed, SlopeSeed], sigma=None, absolute_sigma=False, check_finite=True, bounds=bounds, method=None, jac=None)

    delay, slope = popt
    return slope, delay


def FitDelayStartIntegrator(x, y, DelaySeed, SlopeSeed, fitpoints = 100):
    '''
    Take an x & y array, fit a flat delay followed by a line

    Get initial Y from seperate algo


    Box car average all data, reduce size of boxcar once the minimum has been reached, repeat until min boxcar
    '''
    initialY = FitInitialY(x, y, fitpoints)

    popt, pcov = sp.optimize.curve_fit(DelayIntegrator, x, y, p0=[DelaySeed, SlopeSeed], sigma=None, absolute_sigma=False, check_finite=True, bounds=(-100, 100), method=None, jac=None)

    delay, slope = popt
    return initialY, slope, delay


def MakeCharacterizationPlot(name, time, sig, PlantDelay, PlantSlope, InitialValue, SignalLims = None, save = False):
    plt.clf()
    plt.xlabel("Time (s)")
    plt.ylabel("Temperature")
    plt.plot(time, sig, "b,")

    if SignalLims is not None:
        plt.xlim(SignalLims[0])
        plt.ylim(SignalLims[1])
    ylims = plt.ylim()
    xlims = plt.xlim()

    plt.plot(xlims, [CalcPositionOnLine(PlantSlope, InitialValue, PlantDelay, xlims[0]), CalcPositionOnLine(PlantSlope, InitialValue, PlantDelay, xlims[1])], 'r--')
    plt.plot(xlims, [InitialValue, InitialValue], 'g--')

    plt.plot([0,0],ylims, 'k', linewidth = 0.2)
    plt.plot([xlims[0], xlims[0]],ylims, 'r--')
    plt.plot([PlantDelay, PlantDelay],ylims, 'r--')
    if(save):
        plt.savefig(name + "_Characterization.pdf")

def MakeStabilityPlot(name, update_rate, PlantDelay, PlantSlope, InitialValue, PhaseRange = (-150, -90), save = False):
    omega = np.linspace(1e-5, 100, 10000)
    #f = np.linspace(1e-5, 0.5, 10000)
    freq = [om/(2*pi) for om in omega]

    #Plot data
    cont = System(Plant(PlantDelay, PlantSlope), update_rate)


    #Bode data
    __, mag, phase = cont.makeBode(w = omega)

    #Plotting
    fig = plt.figure()

    ax1, ax2 = fig.subplots(nrows = 2, sharex = True)
    #fig.canvas.resize(2500, 2500)
    #ax1.semilogy()
    ax2.semilogx()
    ax1.semilogx()


    ax1.plot(freq, mag, label="Magnitude (dB)")
    ax1.plot([0,max(freq)], [0,0], label="Unity Gain")
    ax1.set_ylabel("Magnitude (dB)")

    ax2.plot(freq, phase, label="Bode")
    ax2.set_ylabel("Phase")
    ax2.set_ylim(PhaseRange)

    ax2.set_xlabel("f (Hz)");
    #plt.legend();
    #plt.ylim(ylims);
    #fig.set_tight_layout(True)
    ax1.set_title("Bode Plot")
    if(save):
        plt.savefig(name + "_BodePlot.pdf")


def PrintSummary(update_rate, PlantDelay, PlantSlope):
    cont = System(Plant(PlantDelay, PlantSlope), update_rate)
    mpm, pm, freq = cont.phaseMargin()
    print("Phase Margin @ {:0.2}Hz: {:0.3}\nMax Phase Margin: {:0.3}".format(freq, pm, mpm))
    print("Kp, Ti, Ki = {:0.4}, {:0.4}, {:0.4}".format(cont.kp, cont.ti, cont.ki))
    print("1/Kp, 1/Ki = {}, {}".format((1/cont.kp), (1/cont.ki)))

    '''
    kp_m, kp_s, kp_e = findMultShift(cont.kp)
    ki_m, ki_s, ki_e = findMultShift(cont.ki/cont.update_rate)
    print("Kp_m {}, Kp_s {}, Kp_e {:.4}, Set {}".format(kp_m, kp_s, kp_e, kp_m/(2**kp_s)))
    print("Ki_m {}, Ki_s {}, Ki_e {:.4}, Set {}".format(ki_m, ki_s, ki_e, ki_m/(2**ki_s)))
    '''

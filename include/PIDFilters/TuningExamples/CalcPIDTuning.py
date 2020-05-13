from matplotlib import pyplot as plt, style
import sys, os
#print(style.available)
style.use("seaborn-notebook")

from PIDTuner import *
#plantSlope = 1.1
#plantDelay = 1.1


if __name__ == "__main__":
    try:
        fname = sys.argv[1]
        if(not os.path.exists(fname)):
            raise FileNotFoundError
    except:
        print("Input file not found")


    BumpTestPreMeasurements = 2000
    SignalLims = ((0, 7), (0, 3.3))
    #SignalLims = None#((0, 3), (0, 3.3))
    #PlotLims = ((-1, 2), (0, 3.3))
    PlotLims = None#((-1, 2), (0, 3.3))

    ScaleFactor = 3.3/(1<<12)
    time, sig, offset = LoadTestData(fname = fname)
    
    ajustedTime = []
    time0 = time[BumpTestPreMeasurements]
    
    for i, t in enumerate(time):
        ajustedTime.append(time[i] - time0)
    
    ajustedSig = []
    for dp in sig:
        ajustedSig.append(float(dp)*ScaleFactor)
        
    #trimmedSig = np.array(ajustedSig[:len(trimmedTime)])
    trimmedTime = []
    trimmedSig = []
    for t, s in zip(ajustedTime, ajustedSig):
        if(t >= SignalLims[0][0] and t<= SignalLims[0][1]):
            trimmedTime.append(t)
            trimmedSig.append(s)
        
    InitialValue, PlantSlope, PlantDelay = FitDelayIntegrator(trimmedTime, trimmedSig, 0.1, 0.021, fitpoints = 10)
    print(InitialValue, PlantSlope, PlantDelay)
    
    MakeCharacterizationPlot(name = fname.split('.')[0] + "_AutoTuned", time = ajustedTime, sig = ajustedSig, PlantDelay = PlantDelay, PlantSlope = PlantSlope, InitialValue = InitialValue, SignalLims = PlotLims)
    plt.show()
    MakeStabilityPlot(name = fname.split('.')[0] + "_AutoTuned", time = ajustedTime, sig = ajustedSig, PlantDelay = PlantDelay, PlantSlope = PlantSlope, InitialValue = InitialValue, PhaseRange = (-150, -90), SignalLims = PlotLims)

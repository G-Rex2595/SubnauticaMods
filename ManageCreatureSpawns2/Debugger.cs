using System;
using System.Collections.Generic;
using System.Text;
using HarmonyLib;
using BepInEx.Logging;
using System.Threading;

namespace ManageCreatureSpawns2
{
    class Debugger
    {
        private static ManualLogSource log = Logger.CreateLogSource("ManageCreatureSpawns2.Debugger");
        private static ConfigService configService = ConfigService.Instance;
        private static HashSet<string> creaturesFound = new HashSet<string>();

        private static readonly Mutex randomMutex = new Mutex();
        public static Random rEngine = new Random();

        private static readonly Mutex statsMutex = new Mutex();
        private static Dictionary<string, HashSet<Creature>> creatureMap = new Dictionary<string, HashSet<Creature>>();
        private static Dictionary<string, int> creaturesKilled = new Dictionary<string, int>();

        [HarmonyPrefix]
        [HarmonyPriority(Priority.First)]
        public static bool Invoke(string methodName, float time)
        {
            log.LogDebug("Calling Monobehaviour.Invoke");
            return true;
        }

        [HarmonyPrefix]
        [HarmonyPriority(Priority.First)]
        public static bool Evaluate(Creature creature, float time)
        {
            log.LogDebug("Calling StayOnLeashPosition.Evaluate");
            return true;
        }
    }
}
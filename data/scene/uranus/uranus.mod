return {
    -- Uranus barycenter module
    {
        Name = "UranusBarycenter",
        Parent = "SolarSystemBarycenter",
    },

    -- Uranus module
    {   
        Name = "Uranus",
        Parent = "UranusBarycenter",
        Renderable = {
            Type = "RenderablePlanet",
            Frame = "IAU_URANUS",
            Body = "URANUS BARYCENTER",
            Geometry = {
                Type = "SimpleSphere",
                Radius = { 2.5362 , 7 },
                Segments = 100
            },
            Textures = {
                Type = "simple",
                Color = "textures/uranus.jpg",
            },
            Atmosphere = {
                Type = "Nishita", -- for example, values missing etc etc
                MieFactor = 1.0,
                MieColor = {1.0, 1.0, 1.0}
            }
        },
        Transform = {
            Translation = {
                Type = "SpiceEphemeris",
                Body = "URANUS BARYCENTER",
                Observer = "SUN",
                Kernels = "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
            },
            Rotation = {
                Type = "SpiceRotation",
                SourceFrame = "IAU_URANUS",
                DestinationFrame = "ECLIPJ2000",
            },
            Scale = {
                Type = "StaticScale",
                Scale = 1,
            },
        },
    },
    -- UranusTrail module
    {   
        Name = "UranusTrail",
        Parent = "UranusBarycenter",
        Renderable = {
            Type = "RenderableTrail",
            Body = "URANUS BARYCENTER",
            Frame = "GALACTIC",
            Observer = "SUN",
            RGB = {0.60,0.95,1.00 },
            TropicalOrbitPeriod = 30588.740 ,
            EarthOrbitRatio = 83.749,
            DayLength = 17.24,
            Textures = {
                Type = "simple",
                Color = "${COMMON_MODULE}/textures/glare_blue.png",
                -- need to add different texture
            },  
        },
    }
}

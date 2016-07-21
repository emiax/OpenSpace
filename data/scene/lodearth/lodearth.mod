return {
    -- Earth barycenter module
    {
        Name = "EarthBarycenter",
        Parent = "SolarSystemBarycenter",
        Ephemeris = {
            Type = "Static"
        }
    },
    -- EarthTrail module
    {   
        Name = "EarthTrail",
        Parent = "SolarSystemBarycenter",
        Renderable = {
            Type = "RenderableTrail",
            Body = "EARTH",
            Frame = "GALACTIC",
            Observer = "SUN",
            RGB = { 0.5, 0.8, 1.0},
            TropicalOrbitPeriod = 365.242,
            EarthOrbitRatio = 1,
            DayLength = 24
        },
        GuiName = "/Solar/EarthTrail"
    },
    -- RenderableGlobe module
    {
        Name = "LodEarth",
        Parent = "EarthBarycenter",
        Ephemeris = {
            Type = "Spice",
            Body = "EARTH",
            Reference = "ECLIPJ2000",
            Observer = "SUN",
            Kernels = {
                "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
            }
        },
        Rotation = {
            Type = "Spice",
            Frame = "IAU_EARTH",
            Reference = "ECLIPJ2000"
        },
        Renderable = {
            Type = "RenderableGlobe",
            Frame = "IAU_EARTH",
            Body = "EARTH",
            Radii = {6378137.0, 6378137.0, 6356752.314245}, -- Earth's radii
            CameraMinHeight = 300,
            InteractionDepthBelowEllipsoid = 0, -- Useful when having negative height map values
            SegmentsPerPatch = 64,
            TextureInitData = {
                ColorTextureMinimumSize = 512,--512,
                OverlayMinimumSize = 512,
                HeightMapMinimumSize = 64,
            },
            Textures = {
                ColorTextures = {
                    {
                        Type = "Temporal",
                        Name = "Temporal VIIRS SNPP",
                        FilePath = "map_service_configs/earth/Temporal_VIIRS_SNPP_CorrectedReflectance_TrueColor.xml",
                    },
                    {
                        Type = "SingleImage",
                        Name = "Debug Tiles",
                        FilePath = "textures/test_tile.png",
                    },
                    {
                        Type = "Temporal",
                        Name = "Temporal MODIS Aqua CorrectedRecflectance TrueColor",
                        FilePath = "map_service_configs/earth/Temporal_MODIS_Aqua_CorrectedReflectance_TrueColor.xml",
                    },
                    {
                        Name = "MODIS_Terra_CorrectedReflectance_TrueColor",
                        FilePath = "map_service_configs/earth/MODIS_Terra_CorrectedReflectance_TrueColor.xml",
                    },
                    {
                        Name = "ESRI Imagery World 2D",
                        FilePath = "map_service_configs/earth/ESRI_Imagery_World_2D.wms",
                        Enabled = true,
                    }
                },
                GrayScaleOverlays = {
                   
                },
                NightTextures = {
                    {
                        Name = "Earth at Night 2012",
                        FilePath = "map_service_configs/earth/VIIRS_CityLights_2012.xml",
                    },
                },
                WaterMasks = {
                    {
                        Name = "MODIS_Water_Mask",
                        FilePath = "map_service_configs/earth/MODIS_Water_Mask.xml",
                    },
                },
                Overlays = {
                    {
                        Name = "Coastlines",
                        FilePath = "map_service_configs/earth/Coastlines.xml",
                    },
                    {
                        Name = "Reference_Features",
                        FilePath = "map_service_configs/earth/Reference_Features.xml",
                    },
                    {
                        Name = "Reference_Labels",
                        FilePath = "map_service_configs/earth/Reference_Labels.xml",
                    },
                },
                HeightMaps = {
                    {
                        Name = "Terrain tileset",
                        FilePath = "map_service_configs/earth/TERRAIN.wms",
                        Enabled = true,
                    },
                },
                HeightMapOverlays = {

                },
            },
        },
        GuiName = "/Solar/Planets/LodEarth"
    },
}
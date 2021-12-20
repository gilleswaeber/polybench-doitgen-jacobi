library(remotes)

remotes::install_version("Rttf2pt1", version = "1.3.8")

library(extrafont)

extrafont::font_import()
extrafont::loadfonts()

#Puis, uniquement utiliser extrafont::loadfonts() dans les scripts R.
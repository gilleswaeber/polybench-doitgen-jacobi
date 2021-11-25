library(here)
library(ggplot2)
library(tidyverse)
library(dplyr)

data_layout <- c("benchmark_type", "nr", "nq", "np", "rank", "num_cores", "id", "time", "overhead")
cores_only_layout <- c("benchmark_type", "nr", "nq", "np", "num_cores") #, "rank")
rank_layout <- c("benchmark_type", "nr", "nq", "np", "num_cores", "rank", "time")

#setwd(here())
current_path <- "/home/quentin/Desktop/dphpc-project/plots" #here()
setwd("/home/quentin/Desktop/dphpc-project/plots")

print(paste("current path = ", current_path))

#set_here(path='..')

parent_path <- "/home/quentin/Desktop/dphpc-project" #here()
doitgen_benchmark_binary_path <- paste(parent_path, "/build/doitgen/benchmark/", sep="")

#path_from_here_to_R <- paste(here(), "/R/", sep="")
path_from_here_to_R <- paste(parent_path, "/R/", sep="")

print(paste("current path = ", current_path))
print(paste("parent path = ", parent_path))
print(paste("R path = ", path_from_here_to_R))
print(paste("doitgen benchmark path = ", doitgen_benchmark_binary_path ))

source(paste(path_from_here_to_R, "utils.R", sep=""))
source(paste(path_from_here_to_R, "stats.R", sep=""))
source(paste(path_from_here_to_R, "aes.R", sep=""))
source(paste(path_from_here_to_R, "lsbpp_fun.R", sep=""))

data.bh_study <- ReadAllFilesInDir.Aggregate(dir.path="/home/quentin/Desktop/dphpc-project/build/doitgen/benchmark/data/", col=data_layout)
re.bh_study <- CalculateDataSummary(data=data.bh_study, measurevar="time", groupvars=cores_only_layout, conf.interval=.95, quantile.interval=.95)

summary.plot.labels <- c("simple")
aes.var <- aes(x=num_cores, y=median/1000, ymin=CI.NNorm.high/1000, ymax=CI.NNorm.low/1000, color=factor(benchmark_type), shape=factor(benchmark_type))

thread_16 <- subset(data.bh_study, num_cores==4)

print(re.bh_study["benchmark_type"]) 
val <- lsb_shapiro(thread_16, "time")

svg("/home/quentin/Desktop/dphpc-project/plots/myplot.svg")#, width = 350, height = 350)

plot_lat <- ggplot(data=re.bh_study, aes.var)+
  geom_point(size=na_geopoint_size)+ #na_geopoint_size)+  
  geom_line(size=na_geomline_size)+
  geom_errorbar(width = na_geom_errorbar_width, lwd=na_geom_errorbar_width, color="black")+
  scale_color_discrete(name="", labels=summary.plot.labels)+
  scale_shape_discrete(name="",labels=summary.plot.labels)+
  scale_x_continuous(name="# processes", breaks = re.bh_study$num_cores)+
  scale_y_continuous(name="time [ms]")+
  theme_bw(na_theme_size)+
  theme(legend.position="bottom", legend.direction = "horizontal", legend.key = element_rect(fill = "transparent", colour = "transparent"), legend.background = element_rect(fill="transparent", colour="transparent")) + 
  theme(plot.margin=na_plot_margin) + theme(text = element_text(size=na_theme_text_size)) + theme(legend.key.height=na_theme_legend_key_height) +
  theme(axis.title.y=element_text(vjust=na_theme_y_vjust)) +
  theme(axis.title.x=element_text(vjust=na_theme_x_vjust)) +
  labs(title = "doitgen mpi running times", subtitle = paste(re.bh_study$nr, paste(re.bh_study$nq, re.bh_study$np, sep="x"), sep="x"))

print(plot_lat)


dev.off()

#for the box plot
data.bh_study <- ReadAllFilesInDir.Aggregate(dir.path="/home/quentin/Desktop/dphpc-project/build/doitgen/benchmark/data/", col=data_layout)
re.bh_study <- CalculateDataSummary(data=data.bh_study, measurevar="time", groupvars=rank_layout, conf.interval=.95, quantile.interval=.95)

summary.plot.labels <- c("simple")
aes.var <- aes(x=rank)

cores_4 <- subset(data.bh_study, num_cores==4)
ok <- CalculateDataSummary(data=cores_4, measurevar="time", groupvars=c("benchmark_type", "nr", "nq", "np", "num_cores", "time"), conf.interval=.95, quantile.interval=.95)

plot_box <- ggplot(data=cores_4, aes(x=rank)) +
  geom_histogram(binwidth=1, colour="black", position="dodge") +
  scale_fill_identity()

print(plot_box)
#




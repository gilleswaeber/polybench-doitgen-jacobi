library(here)
library(ggplot2)
library(tidyverse)
library(dplyr)

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

ReadAllRiInDir.Aggregate <- function(dir.path=NA, col=NA, del.num=1){
  dir.files <- list.files(path=dir.path)
  table <- NULL
  for(file in dir.files){
    file.path <- paste( dir.path, file , sep="")
    in.table <- read.table( file.path , header=TRUE, col.names=col)
    in.table <- in.table[-(del.num),]
    rownames(in.table) <- NULL
    table <- rbind(table, in.table) 
  }
  return(table)
}


#.Aggregate
data.bh_study <- ReadAllFilesInDir.Aggregate(dir.path="/home/quentin/Desktop/dphpc-project/build/doitgen/benchmark/data/", col=c("rank", "benchmark_type", "nr", "nq", "np", "runs", "processes", "id", "time", "overhead"))


#re.bh_study <- CalculateDataSummary(data=data.bh_study, measurevar="time", groupvars=c("rank", "benchmark_type", "nr", "nq", "np", "runs", "processes"), conf.interval=.95, quantile.interval=.95)
test <-data.bh_study %>% group_by(processes)


#This is not useful in our case but might be if we do a more comprehensive banchmark
#For instance we could benchmark the matrix multiply time for each thread
#re.bh_study.toplot <- subset(re.bh_study)

summary.plot.labels <- c("Runtime")
aes.var <- aes(x=threads, y=median/1000000, ymin=CI.NNorm.high/1000000, ymax=CI.NNorm.low/1000000)


thread_16 <- subset(data.bh_study, threads==16)

val <- lsb_shapiro(thread_16, "time")

plot_lat <- ggplot(data=re.bh_study, aes.var) +
  geom_point(size=na_geopoint_size)+  
  geom_line(size=na_geomline_size)+
  geom_errorbar(width = na_geom_errorbar_width, lwd=na_geom_errorbar_width, color="black")+
  scale_color_discrete(name="", labels=summary.plot.labels)+
  scale_shape_discrete(name="",labels=summary.plot.labels)+
  scale_x_continuous(name="Threads")+
  scale_y_continuous(name="Time")+
  theme_bw(na_theme_size) +
  theme(legend.position="bottom", legend.direction = "horizontal", legend.key = element_rect(fill = "transparent", colour = "transparent"), legend.background = element_rect(fill="transparent", colour="transparent")) + 
  theme(plot.margin=na_plot_margin) + theme(text = element_text(size=na_theme_text_size)) + theme(legend.key.height=na_theme_legend_key_height) +
  theme(axis.title.y=element_text(vjust=na_theme_y_vjust)) +
  theme(axis.title.x=element_text(vjust=na_theme_x_vjust))

print(plot_lat)

sink("filename.txt")  # to be found in dir `getwd()`
cat(paste(val[[3]],"\n"))
cat(paste("W : ", val[[1]], "\n"))
cat(paste("p : ", val[[2]], "\n"))
sink()


require(rmarkdown)
my_text <- readLines("filename.txt")
cat(my_text, sep="  \n", file = "markdown_text.Rmd")
render("markdown_text.Rmd", pdf_document())
file.remove("markdown_text.Rmd")

pdf("graph.pdf")
print(plot_lat)
dev.off()

#https://stackoverflow.com/questions/13273611/how-to-append-a-plot-to-an-existing-pdf-file
outFileName <- "Results.pdf"
system2(command = "pdftk",
        args = c("markdown_text.pdf", "graph.pdf", "cat output", shQuote(outFileName)))


#PrintGGPlotOnPDF(plot_lat, "test.pdf")


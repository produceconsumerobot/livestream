% function LivestreamPlotData( dateRanges, colorOrder )
function LivestreamPlotData( dateRanges, reportFigBool, colorOrder )

if ~exist('colorOrder', 'var') | isempty(colorOrder)
    colorOrder = get(gca,'ColorOrder');
end


thresholds = [200, 600, 200, 600, 600, 300, 600, 600, 600];

writeDir = 'LivestreamFigures\';
writeFileBase = 'Livestream';

% ----------------------------------------------------------------------- %
% Plotting
% ----------------------------------------------------------------------- %
try
for d = 1:size(dateRanges, 1)
    % Temperature Data
    startDate = dateRanges{d, 1};
    endDate = dateRanges{d, 2};
    tagType = 'TN';
    nPis = 10;
    % Load the data
    [temps piNames] = LivestreamLoadData(tagType, nPis, startDate, endDate);
    % Calculate which pis are alive
    s = min(temps.datetime.data);
    e = max(temps.datetime.data);
    g = datetime(year(s), month(s), day(s), 0, 0, 0) : datetime(year(e), month(e), day(e) + 1);
    alive = zeros(length(g), nPis);
    for j=1:length(g)-1
        % Look for (realistic) changes in temp reading to determine aliveness
        diffTemp = diff(temps.data(temps.datetime.data > g(j) & temps.datetime.data < g(j+1),:));
        alive(j, :) = squeeze(sum(abs(diffTemp) > 0 & abs(diffTemp) < 10, 1)) > 0;
    end
    %piNames = {'M';'L13';'L17';'L11';'L14';'L16';'L21';'L22';'L19';'L18'};
    figure(1)
    clf;
    set(gcf,'DefaultAxesColorOrder',cat(1, colorOrder, [0, 0, 0; 0.5, 0.5, 0.5; 1, 0, 1]));
    plot(temps.datetime.data, temps.data);
    title('Livestream Temperature');
    ylabel('Temperature (C)');
    xlabel('date/time');
    set(gca, 'ylim', [min(min(temps.data(temps.data > -60))) max(max(temps.data))]);
    grid on;
    FigureLegend(piNames);
    set(gcf, 'units', 'inches', 'position', [0 0 20 5]);
    
    if reportFigBool
        suffix = '01_temp';
        writePath = sprintf('%s%s_%02d%02d%02d-%02d%02d%02d_%s.png', ...
            writeDir, writeFileBase, ...
            year(startDate), month(startDate), day(startDate), ...
            year(endDate), month(endDate), day(endDate),...
            suffix);
        fprintf('Writing: %s', writePath);
        mkdir(writeDir)
        print(gcf, '-dpng', writePath,'-r100');
        %ReportFigSM(gcf, ['LivesteamFigs\'], {'LivestreamData'});
    end

    figure(2);
    clf;
    set(gcf,'DefaultAxesColorOrder',cat(1, colorOrder, [0, 0, 0; 0.5, 0.5, 0.5; 1, 0, 1]));
    colormap('winter');
    imagesc(alive(1:end-1,:)');
    set(gca, 'yticklabel', piNames);
    set(gca, 'xtick', 1:length(g));
    set(gca, 'xticklabel', datestr(g, 'mm/dd'));
    ylabel('Pi');
    xlabel('Date');
    title('Livestream Up By Day');
    FigureLegend({'Down'; 'Up'}, {'b'; 'g'});
    set(gcf, 'units', 'inches', 'position', [0 0 20 5]);
    if reportFigBool
        suffix = '02_up';
        writePath = sprintf('%s%s_%02d%02d%02d-%02d%02d%02d_%s.png', ...
            writeDir, writeFileBase, ...
            year(startDate), month(startDate), day(startDate), ...
            year(endDate), month(endDate), day(endDate),...
            suffix);
        fprintf('Writing: %s', writePath);
        mkdir(writeDir)
        print(gcf, '-dpng', writePath,'-r100');
        %ReportFigSM(gcf, ['LivesteamFigs\'], {'LivestreamData'});
    end
    
    % Interaction Data
    startDate = dateRanges{d, 1};
    endDate = dateRanges{d, 2};
    tagType = 'LD';
    nPis = 9;

    
    [dists piNames] = LivestreamLoadData(tagType, nPis, startDate, endDate);
    % Average the distance and sum the interactions by hour
    s = min(dists.datetime.data);
    e = max(dists.datetime.data);
    h = datetime(year(s), month(s), day(s), hour(s), 0, 0) : 1/24 : datetime(year(e), month(e), day(e) + 1);
    distCounts = zeros(length(h), nPis);
    %distMeans = zeros(length(h), nPis);
    distMin = 20; % min distance to be considered an interaction
    for j=1:length(h)-1
        temp = dists.data(dists.datetime.data > h(j) & dists.datetime.data < h(j+1),:);
        for k=1:nPis
            distCounts(j,k) = sum(temp(:,k) < thresholds(k) & temp(:,k) > distMin,1);
        end
        %distCounts(j,:) = temp 
        %distMeans(j,:) = mean(temp, 1);
    end

    s = min(dists.datetime.data);
    e = max(dists.datetime.data);
    g = datetime(year(s), month(s), day(s), 0, 0, 0) : datetime(year(e), month(e), day(e) + 1);
    ixCountPerDay = zeros(length(g), nPis);
    %ixCountPerDay = zeros(length(h), 1); 
    for j=1:(length(g)-1)
        temp = dists.data(dists.datetime.data > g(j) & dists.datetime.data < g(j+1),:);
        for k=1:nPis
            ixCountPerDay(j,k) = sum(temp(:,k) < thresholds(k) & temp(:,k) > distMin,1);
        end
    end
    %piNames = {'L13';'L17';'L11';'L14';'L16';'L21';'L22';'L19';'L18'};
    figure(3);
    clf;
    set(gcf,'DefaultAxesColorOrder',cat(1, colorOrder, [0, 0, 0; 0.5, 0.5, 0.5; 1, 0, 1]));
    subplot(2, 1, 1);
    hold on
    %set(gca,'ColorOrder', cat(1, get(gca,'ColorOrder'), [0, 0, 0]));
    plot(h, distCounts);
    for j=1:(length(g)-1)
        mco = get(gca,'ColorOrder');
        yLim = get(gca, 'ylim');
        textY = yLim(1) + 0.95 * (yLim(2) - yLim(1));
        text(datenum(g(j) + 0.1), textY, num2str(sum(ixCountPerDay(j,:),2)), 'color', mco(end,:));
        %FigureLegend({num2str(sum(ixCountPerDay(j,:),2))}, {mco(end,:)}, (j-0.4)/(size(ixCountPerDay, 1)+1.5), .9, 0.1);
    end
    title('Livestream Interaction');
    ylabel('Interactions Per Hour');
    xlabel('date/time');
    FigureLegend(cat(1, piNames, {'ALL'}), [], 1.05);
    yyaxis right;
    plot(g(1:end-1)+0.5, sum(ixCountPerDay(1:end-1,:),2), 'linewidth', 2);
    ylabel('Interactions Per Day')
    grid on;

    subplot(2, 1, 2);
    grid on;
    plot(dists.datetime.data, dists.data, '.');
    set(gca, 'ylim', [0 max(thresholds)*2 + 100]);
    %plot(h, distMeans);
    ylabel('Interaction Distance (cm)');
    xlabel('date/time');
    FigureLegend(piNames);
    grid on;
    set(gcf, 'units', 'inches', 'position', [-2 0 25 10]);
    if reportFigBool
        suffix = '03_IX';
        writePath = sprintf('%s%s_%02d%02d%02d-%02d%02d%02d_%s.png', ...
            writeDir, writeFileBase, ...
            year(startDate), month(startDate), day(startDate), ...
            year(endDate), month(endDate), day(endDate),...
            suffix);
        fprintf('Writing: %s', writePath);
        mkdir(writeDir)
        print(gcf, '-dpng', writePath,'-r100');
        %ReportFigSM(gcf, ['LivesteamFigs\'], {'LivestreamData'});
    end
    
    figure(4);
    clf;
    set(gcf,'DefaultAxesColorOrder',cat(1, colorOrder, [0, 0, 0; 0.5, 0.5, 0.5; 1, 0, 1]));
    imagesc(ixCountPerDay(1:end-1,:)');
    set(gca, 'yticklabel', piNames);
    set(gca, 'xtick', 1:length(g));
    set(gca, 'xticklabel', datestr(g, 'mm/dd'));
    ylabel('Pi');
    xlabel('Date');
    title('Livestream Interactions Per Day');
    colorbar;
    set(gcf, 'units', 'inches', 'position', [0 0 20 05]);
    if reportFigBool
        suffix = '04_IXperDay';
        writePath = sprintf('%s%s_%02d%02d%02d-%02d%02d%02d_%s.png', ...
            writeDir, writeFileBase, ...
            year(startDate), month(startDate), day(startDate), ...
            year(endDate), month(endDate), day(endDate),...
            suffix);
        fprintf('Writing: %s', writePath);
        mkdir(writeDir)
        print(gcf, '-dpng', writePath,'-r100');
        %ReportFigSM(gcf, ['LivesteamFigs\'], {'LivestreamData'});
    end
end
% END Plotting
% -------------------------------------------------------------------- %
catch
    e = lasterror
    keyboard
end
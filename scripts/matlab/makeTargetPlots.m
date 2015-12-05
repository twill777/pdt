%Process multiple files into plots
function makeTargetPlots(type)
    %Interp spacing:
    interpTime = 0.001;

    %Colours (https://kuler.adobe.com):
    desat = 0.9;
    y = desat*[1 1 0]; %[127 127 0]/255; %[1 1 0];
    m = desat*[1 0 1]; %[127 0 127]/255; %[1 0 1];
    c = desat*[0 1 1]; %[0 127 127]/255; %[0 1 1];
    r = desat*[1 0 0]; %[127 0 0]/255; %[1 0 0];
    g = desat*[0 1 0]; %[0 127 0]/255; %[0 1 0];
    b = desat*[0 0 1]; %[0 0 127]/255; %[0 0 1];
    w = [1 1 1];
	k = [0 0 0];
    
    %Plot time:
    if (strcmpi(type,'Tol'))
        useMedian = true;
        
        %Number of experiments in each file:
        numExp = 100;
        
        %The relative margin above the medians that we're willing to plot...
        yLimMargin = 0.25;
        
        %The type of targeted problem this is
        variateType = 'Tol'; %'Map', 'Gap'

        %The filesnames ommit the data/ and the .csv.
        filenames = {'R2S3880217150TimeVTarget';
                     'R4S1770665411TimeVTarget'
                     'R8S4232850725TimeVTarget'};
        firstTarget = 1;
       
        %The planner names and colours:
        planners = { 'RRT*', k;
                     'RRT* w/ pruning', r;
                     'RRT* w/ new reject', m;
                     'RRT* w/ sample reject', c;
                     'RRT* trio', b;
                     'Informed RRT*', g};
       
       ignorePlanners = {};
    elseif (strcmpi(type,'Map'))
        useMedian = true;
        
        %Number of experiments in each file:
        numExp = 100;
        
        %The relative margin above the medians that we're willing to plot...
        yLimMargin = 0.25;
        
        %The type of targeted problem this is
        variateType = 'Map'; %'Tol', 'Gap'

        %The filesnames ommit the data/ and the .csv.
        filenames = {'R2S750588957TimeVMap';
                     'R4S1310937456TimeVMap'
                     'R8S576679592TimeVMap'};
        firstTarget = 1;
       
        %The planner names and colours:
        planners = { 'RRT*', k;
                     'RRT* w/ pruning', r;
                     'RRT* w/ new reject', m;
                     'RRT* w/ sample reject', c;
                     'RRT* trio', b;
                     'Informed RRT*', g};
       
       ignorePlanners = {};
    else
        error('Unhandled argument');
    end

    for i = 1:size(filenames,1)
        data = processTargetData(['data/' filenames{i} '.csv'], numExp, firstTarget);
        [succHandl, timeHandl] = plotTargetTimes(variateType, data, planners(:,1), planners(:,2), ignorePlanners, useMedian, yLimMargin);

        saveEpsToPdf(succHandl, ['data/' filenames{i} '_success.pdf']);
        close(succHandl)

        saveEpsToPdf(timeHandl, ['data/' filenames{i} '_time.pdf']);
        close(timeHandl)
    end
end
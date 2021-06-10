//CREDITS TO PEPIJN VERBURG, WHO WROTE A SIGNIFICANT PART OF THIS CODE

// CHANGE to 50000 or something after update of Mathias to support a lot of data, keep track of your dataset to see if this is enough
const maxDataAmount = 80000;
// CHANGE this to the dataset ID found in the URL when you have your dataset opened in a browser. 
const fitbitDataSet = 1254;
// CHANGE this to the wearable_id field found when downloading your dataset.
const wearableId = 247;
const dataStartDate = new Date();
dataStartDate.setDate(dataStartDate.getDate() - 7);
dataStartDate.setHours(0, 0, 0, 0);
// reference: https://data.id.tue.nl/documentation/scripts
const stats = DF.dataset(1254).filter(
  wearableId, 
  maxDataAmount,
  dataStartDate.getTime(), 
  (new Date()).getTime()
).stats('step_count');
const totalSteps = stats.step_count.sum;
const goalSteps = 100000;
let progressPosition = Math.round((totalSteps / goalSteps)*800);
DF.print("Total steps: "+ totalSteps);
const wiggleState = { 
  //progressPos: 400,
  levelInterv: 2,
  intState: 1,
};
// CHANGE to your day of deployment so we can calculate the third days
const studyStartTime = (new Date('2021-05-17T07:30:00')).getTime();
const daysPassed = Math.floor(((new Date().getTime()) - studyStartTime) / (1 * 24 * 60 * 60 * 1000));
const isThirdDay = (daysPassed % 3) === 0;
if (isThirdDay) {
  DF.print(wiggleState);
  DF.print('Sent out WiggleState OOCSI message!');
  //DF.print(data.activateScript);
} else {
  DF.oocsi('wigglechannel', wiggleState);
  DF.print('It is not the third day!');
  DF.print(wiggleState);
}

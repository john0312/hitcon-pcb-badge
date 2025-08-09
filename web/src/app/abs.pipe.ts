import { Pipe, PipeTransform } from '@angular/core';

@Pipe({
  name: 'abs'
})
export class AbsPipe implements PipeTransform {
  transform(value: number): number {
    return value < 0 ? -value : value;
  }
}
